// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationModule.h"
#include "BYGLocalizationSettings.h"
#include "BYGLocalization.h"

#include "Internationalization/StringTableRegistry.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

#define LOCTEXT_NAMESPACE "BYGLocalizationModule"

void FBYGLocalizationModule::StartupModule()
{
	Loc = MakeShareable( new UBYGLocalization() );
	Provider = MakeShareable( new UBYGLocalizationSettingsProvider() );
	Loc->Construct( Provider );

	const UBYGLocalizationSettings* Settings = Provider->GetSettings();

	bool bDoUpdate = false;
	const bool bHasCommandLineFlag = !Settings->bUpdateLocsWithCommandLineFlag || FParse::Param( FCommandLine::Get(), *Settings->CommandLineFlag );
#if UE_BUILD_SHIPPING
	if ( Settings->bUpdateLocsInShippingBuilds )
	{
		bDoUpdate = true && bHasCommandLineFlag;
	}
#else
	if ( Settings->bUpdateLocsInDebugBuilds )
	{
		bDoUpdate = true && bHasCommandLineFlag;
	}
#endif

#if WITH_EDITOR
	if ( bDoUpdate || Settings->bForceUpdateTranslations)
	{
		Loc->UpdateTranslations();
	}
#endif

	CurrentLanguageCode = Settings->PrimaryLanguageCode;
	ReloadLocalizations();
}

void FBYGLocalizationModule::ShutdownModule()
{
	// Using this because GetDefault<UBYGLocalizationSettings>() is not valid inside ShutdownModule
	UnloadLocalizations();

	Loc;
}

void FBYGLocalizationModule::ReloadLocalizations()
{
	UnloadLocalizations();

	// TODO provider
	const UBYGLocalizationSettings* Settings = Provider->GetSettings();

	const TArray<FString> Categories = Settings->LocalizationCategories;
	const TArray<FBYGLocaleInfo> Entries = Loc->GetAvailableLocalizations(CurrentLanguageCode);

	for (const FString Category : Categories)
	{
		bool Found = false;
		for (FBYGLocaleInfo Entry : Entries)
		{
			UE_LOG(LogBYGLocalization, Verbose, TEXT("Entry: %s / %s / %s / %s"), *Entry.LocaleCode, *Entry.LocalizedName.ToString(), *Entry.Category, *Entry.FilePath);

			if (/*Entry.LocaleCode == CurrentLanguageCode && */Entry.Category == Category)
			{
				UE_LOG(LogBYGLocalization, Verbose, TEXT("[ReloadLocalizations] Load Localization file: %s"), *Entry.FilePath);
				StringTableIDs.Add(FName(*Category));
				FStringTableRegistry::Get().UnregisterStringTable(StringTableIDs[StringTableIDs.Num() - 1]);
				FStringTableRegistry::Get().Internal_LocTableFromFile(StringTableIDs[StringTableIDs.Num() - 1], Category, Entry.FilePath, FPaths::ProjectContentDir());
				Found = true;
				break;
			}
		}

		if (Found)
			continue;

		//StringTableIDs.Add( FName( *Settings->StringtableID ) );
		//FStringTableRegistry::Get().Internal_LocTableFromFile( StringTableIDs[ 0 ], Settings->StringtableNamespace, Filename, FPaths::ProjectContentDir() );

		// We don't want to register this when we're in editor, because we don't want the 'en' language to be shown when selecting FText in Blueprints
	#if !WITH_EDITOR
		// We always keep the localization for the Primary language in memory and use it as a fallback in case a string is not found in another language
		{
			FString Filename = Loc->GetFileWithPathFromLanguageCode("en", Category);
			Filename = Filename.Replace(TEXT("/Game/"), TEXT(""));
			UE_LOG(LogBYGLocalization, Verbose, TEXT("[ReloadLocalizations] Load FALLOUT Localization file: %s"), *Filename);
			StringTableIDs.Add( FName( *Category) );
			FStringTableRegistry::Get().Internal_LocTableFromFile( StringTableIDs[StringTableIDs.Num()-1], Category, Filename, FPaths::ProjectContentDir() );
		}
	#endif

	}

}

void FBYGLocalizationModule::UpdateTranslations()
{
	if (Loc.IsValid())
	{
		Loc->UpdateTranslations();
	}
}

void FBYGLocalizationModule::UnloadLocalizations()
{
	// Using this because GetDefault<UBYGLocalizationSettings>() is not valid inside ShutdownModule
	for ( const FName& ID : StringTableIDs )
	{
		FStringTableRegistry::Get().UnregisterStringTable( ID );
	}
	StringTableIDs.Empty();
}

void FBYGLocalizationModule::AddReferencedObjects( FReferenceCollector& Collector )
{
	//Collector.AddReferencedObject( Loc );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FBYGLocalizationModule, BYGLocalization )

