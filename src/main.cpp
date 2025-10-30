#include "SkeletonViewerApp.h"

int main()
{
    std::shared_ptr<SkeletonViewerApp> app = std::make_shared<SkeletonViewerApp>();
    if (!app->Init())
    {
        return -1;
    }

    app->Run();
    app->Destroy();

    return 0;
}
