// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "UnrealEd.h"
#include "Engine.h"

class FBYGLocalizationEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnEndPIE(const bool bSimulate);

protected:
	bool HandleSettingsSaved();

};
