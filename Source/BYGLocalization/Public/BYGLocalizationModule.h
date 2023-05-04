// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Public/Modules/ModuleManager.h"
#include "UObject/GCObject.h"

class FBYGLocalizationModule : public IModuleInterface, public FGCObject
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool SupportsDynamicReloading() override { return true; }

	void ReloadLocalizations();
	void UpdateTranslations();

	static inline FBYGLocalizationModule& Get()
	{
		static FName ModuleName( "BYGLocalization" );
		return FModuleManager::LoadModuleChecked<FBYGLocalizationModule>( ModuleName );
	}

	virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;

	inline class UBYGLocalization* GetLocalization() { return Loc.Get(); }
	inline FString GetCurrentLanguageCode() { return CurrentLanguageCode; }
	inline void SetCurrentLanguageCode(FString InCurrentLanguageCode) { CurrentLanguageCode = InCurrentLanguageCode; }

protected:
	void UnloadLocalizations();

	// TODO FGCObject
	TSharedPtr<class UBYGLocalization> Loc;

	TArray<FName> StringTableIDs;
	FString CurrentLanguageCode;
};
