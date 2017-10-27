/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2017                                                               *
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

#ifndef __OPENSPACE_CORE___ASSETMANAGER___H__
#define __OPENSPACE_CORE___ASSETMANAGER___H__

#include <openspace/scene/asset.h>

#include <openspace/scene/assetloader.h>
#include <openspace/util/resourcesynchronizer.h>

#include <openspace/scripting/lualibrary.h>

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace openspace {

class Asset;

class AssetManager {
public:
    AssetManager(
        std::unique_ptr<AssetLoader> loader,
        std::unique_ptr<AssetSynchronizer> synchronizer
    );

    enum class AssetState : int {
        Unloaded = 0,
        Loaded = 1,
        Synchronized = 2,
        Initialized = 3
    };

    void update();
    void setTargetAssetState(const std::string& path, AssetState targetState);
    void clearAllTargetAssets();
    std::vector<std::shared_ptr<Asset>> allAssets();
    scripting::LuaLibrary luaLibrary();

private:

    std::unordered_map<std::string, AssetState> _pendingStateChangeCommands;
    std::unordered_map<std::shared_ptr<Asset>, AssetState> _stateChangesInProgress;
    std::unordered_map<std::shared_ptr<Asset>, std::unordered_set<std::shared_ptr<Asset>>> _syncAncestors;
    std::unique_ptr<AssetLoader> _assetLoader;
    std::unique_ptr<AssetSynchronizer> _assetSynchronizer;
};

} // namespace openspace

#endif // __OPENSPACE_CORE___ASSETMANAGER___H__
