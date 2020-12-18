/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2020                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <modules/exoplanets/exoplanetshelper.h>

#include <openspace/util/spicemanager.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/fmt.h>
#include <ghoul/logging/logmanager.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <string_view>
#include <fstream>
#include <sstream>

namespace {
    constexpr const char* _loggerCat = "ExoplanetsModule";

    constexpr const char* BvColormapPath = "${SYNC}/http/stars_colormap/2/colorbv.cmap";
}

namespace openspace::exoplanets {

bool isValidPosition(const glm::vec3& pos) {
    return !glm::any(glm::isnan(pos));
}

bool hasSufficientData(const ExoplanetDataEntry& p) {
    const glm::vec3 starPosition{ p.positionX , p.positionY, p.positionZ };

    bool validStarPosition = isValidPosition(starPosition);
    bool hasSemiMajorAxis = !std::isnan(p.a);
    bool hasOrbitalPeriod = !std::isnan(p.per);

    return validStarPosition && hasSemiMajorAxis && hasOrbitalPeriod;
}

glm::vec3 starColor(float bv) {
    std::ifstream colorMap(absPath(BvColormapPath), std::ios::in);

    if (!colorMap.good()) {
        LERROR(fmt::format(
            "Failed to open colormap data file: '{}'",
            absPath(BvColormapPath)
        ));
        return glm::vec3(0.f, 0.f, 0.f);
    }

    const int t = static_cast<int>(round(((bv + 0.4) / (2.0 + 0.4)) * 255));
    std::string color;
    for (int i = 0; i < t + 12; i++) {
        getline(colorMap, color);
    }
    colorMap.close();

    std::istringstream colorStream(color);
    float r, g, b;
    colorStream >> r >> g >> b;

    return glm::vec3(r, g, b);
}

glm::dmat4 computeOrbitPlaneRotationMatrix(float i, float bigom, float omega) {
    // Exoplanet defined inclination changed to be used as Kepler defined inclination
    const glm::dvec3 ascendingNodeAxisRot = glm::dvec3(0.0, 0.0, 1.0);
    const glm::dvec3 inclinationAxisRot = glm::dvec3(1.0, 0.0, 0.0);
    const glm::dvec3 argPeriapsisAxisRot = glm::dvec3(0.0, 0.0, 1.0);

    const double asc = glm::radians(bigom);
    const double inc = glm::radians(i);
    const double per = glm::radians(omega);

    const glm::dmat4 orbitPlaneRotation =
        glm::rotate(asc, glm::dvec3(ascendingNodeAxisRot)) *
        glm::rotate(inc, glm::dvec3(inclinationAxisRot)) *
        glm::rotate(per, glm::dvec3(argPeriapsisAxisRot));

    return orbitPlaneRotation;
}

glm::dmat3 computeSystemRotation(glm::dvec3 starPosition) {
    const glm::dvec3 sunPosition = glm::dvec3(0.0, 0.0, 0.0);
    const glm::dvec3 starToSunVec = glm::normalize(sunPosition - starPosition);
    const glm::dvec3 galacticNorth = glm::dvec3(0.0, 0.0, 1.0);

    const glm::dmat3 galacticToCelestialMatrix =
        SpiceManager::ref().positionTransformMatrix("GALACTIC", "J2000", 0.0);

    const glm::dvec3 celestialNorth = glm::normalize(
        galacticToCelestialMatrix * galacticNorth
    );

    // Earth's north vector projected onto the skyplane, the plane perpendicular to the
    // viewing vector (starToSunVec)
    const float celestialAngle = static_cast<float>(glm::dot(
        celestialNorth,
        starToSunVec
    ));
    glm::dvec3 northProjected = glm::normalize(
        celestialNorth - (celestialAngle / glm::length(starToSunVec)) * starToSunVec
    );

    const glm::dvec3 beta = glm::normalize(glm::cross(starToSunVec, northProjected));

    return glm::dmat3(
        northProjected.x,
        northProjected.y,
        northProjected.z,
        beta.x,
        beta.y,
        beta.z,
        starToSunVec.x,
        starToSunVec.y,
        starToSunVec.z
    );
}

glm::vec2 computeHabitableZone(float teff, float luminosity) {
    // Kopparapu's formula only considers stars with teff in range [2600, 7200] K.
    // However, we want to use the formula for more stars, so add some flexibility to
    // the teff boundaries
    if (teff > 8000.f || teff < 2000.f) {
        // For the other stars, use a method by Tom E. Morris:
        // https://www.planetarybiology.com/calculating_habitable_zone.html
        float inner = std::sqrt(luminosity / 1.1f);
        float outer = std::sqrt(luminosity / 0.53f);
        return glm::vec2(inner, outer);
    }

    struct Coefficients {
        float seffSun;
        float a;
        float b;
        float c;
        float d;
    };

    // Coefficients for planets of 1 Earth mass. Received from:
    // https://depts.washington.edu/naivpl/sites/default/files/HZ_coefficients.dat
    constexpr const Coefficients coefficients[] = {
        // Inner boundary - Runaway greenhouse
        {1.10700E+00f, 1.33200E-04f, 1.58000E-08f, -8.30800E-12f, -1.93100E-15f},
        // Outer boundary - Maximum greenhouse
        {3.56000E-01f, 6.17100E-05f, 1.69800E-09f, -3.19800E-12f, -5.57500E-16f}
    };

    const float tstar = teff - 5780.f;
    const float tstar2 = tstar * tstar;

    glm::vec2 distances;
    for (int i = 0; i < 2; ++i) {
        const Coefficients& coeffs = coefficients[i];
        float seff = coeffs.seffSun + (coeffs.a * tstar) + (coeffs.b * tstar2) +
                     (coeffs.c * tstar * tstar2) + (coeffs.d * tstar2 * tstar2);

        distances[i] = std::pow(luminosity / seff, 0.5f);
    }

    return distances;
}

std::string createIdentifier(std::string name) {
    std::replace(name.begin(), name.end(), ' ', '_');
    std::replace(name.begin(), name.end(), '.', '-');
    sanitizeNameString(name);
    return name;
}

void sanitizeNameString(std::string& s) {
    // We want to avoid quotes and apostrophes in names, since they cause problems
    // when a string is translated to a script call
    s.erase(remove(s.begin(), s.end(), '\"'), s.end());
    s.erase(remove(s.begin(), s.end(), '\''), s.end());
}

} // namespace openspace::exoplanets
