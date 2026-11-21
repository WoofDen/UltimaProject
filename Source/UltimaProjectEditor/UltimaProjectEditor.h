#pragma once

#include "Modules/ModuleManager.h"

class FUltimaProjectEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
