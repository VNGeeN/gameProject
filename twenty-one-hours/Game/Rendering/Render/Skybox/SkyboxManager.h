#include "SkyboxRenderer.h"
#include <string>
#include <map>
#include <memory>

class SkyboxManager
{
public:
    void addSkybox(const std::string &name, const std::string &filename);
    void setActiveSkybox(const std::string &name);
    void render(float angle, float horizon)
    {
        if (mActive)
            mActive->render(angle, horizon);
    }

private:
    std::map<std::string, std::unique_ptr<SkyboxRenderer>> mSkyboxes;
    SkyboxRenderer *mActive = nullptr;
};