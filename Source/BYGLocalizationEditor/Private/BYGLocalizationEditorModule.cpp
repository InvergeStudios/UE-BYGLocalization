// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationEditorModule.h"

#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"
#include "Developer/Settings/Public/ISettingsContainer.h"

#include "BYGLocalizationEditor/Private/StatsWindow/BYGLocalizationStatsWindow.h"
#include "Framework/Docking/TabManager.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructure.h"
#include "Widgets/Docking/SDockTab.h"

#include "BYGLocalizationSettings.h"
#include "BYGLocalizationUIStyle.h"
#include "BYGLocalizationStatics.h"

#define LOCTEXT_NAMESPACE "BYGLocalizationEditorModule"

// Not sure why this is necessary, saw it in another module
namespace BYGLocalizationModule
{
	static const FName LocalizationStatsTabName = FName( TEXT( "BYG Localization Stats" ) );
}


TSharedRef<SDockTab> SpawnStatsTab( const FSpawnTabArgs& Args )
{
	return SNew( SDockTab )
		//.Icon( FBYGRichTextUIStyle::GetBrush( "BYGRichText.TabIcon" ) )
		.TabRole( ETabRole::NomadTab )
		.Label( NSLOCTEXT( "BYGLocalization", "StatsTabTitle", "BYG Localization Stats" ) )
		[
			SNew( SBYGLocalizationStatsWindow )
		];
}

void FBYGLocalizationEditorModule::StartupModule()
{
	if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer( "Project" );
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings( "Project", "Plugins", "BYG Localization",
			LOCTEXT( "RuntimeGeneralSettingsName", "BYG Localization" ),
			LOCTEXT( "RuntimeGeneralSettingsDescription", "Simple loc system with support for fan translations." ),
			GetMutableDefault<UBYGLocalizationSettings>()
		);

		if ( SettingsSection.IsValid() )
		{
			SettingsSection->OnModified().BindRaw( this, &FBYGLocalizationEditorModule::HandleSettingsSaved );
		}
	}

	FBYGLocalizationUIStyle::Initialize();


	FGlobalTabmanager::Get()->RegisterNomadTabSpawner( BYGLocalizationModule::LocalizationStatsTabName, FOnSpawnTab::CreateStatic( &SpawnStatsTab ) )
		.SetDisplayName( NSLOCTEXT( "BYGLocalization", "TestTab", "BYG Localization Stats" ) )
		.SetTooltipText( NSLOCTEXT( "BYGLocalization", "TestTooltipText", "Open a window with info on localizations." ) )
		.SetGroup( WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory() )
		.SetIcon( FSlateIcon( FBYGLocalizationUIStyle::GetStyleSetName(), "BYGLocalization.TabIcon" ) );


	FEditorDelegates::EndPIE.AddRaw(this, &FBYGLocalizationEditorModule::OnEndPIE);
}

void FBYGLocalizationEditorModule::ShutdownModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" );

	if ( SettingsModule != nullptr )
	{
		SettingsModule->UnregisterSettings( "Project", "Plugins", "BYG Localizations" );
	}
}

bool FBYGLocalizationEditorModule::HandleSettingsSaved()
{
	UBYGLocalizationSettings* Settings = GetMutableDefault<UBYGLocalizationSettings>();
	const bool ResaveSettings = Settings->Validate();

	if ( ResaveSettings )
	{
		Settings->SaveConfig();
	}

	return true;
}



void FBYGLocalizationEditorModule::OnEndPIE(const bool bSimulate)
{
	//"/Localization/loc_en.csv"
	UBYGLocalizationSettings* Settings = GetMutableDefault<UBYGLocalizationSettings>();
	FString Path = Settings->PrimaryLocalizationDirectory.Path;

	if (Path.StartsWith("/Game"))
	{
		Path.Split("Game", nullptr, &Path);
	}

	Path = Path + "/" + Settings->FilenamePrefix + Settings->PrimaryLanguageCode + "." + Settings->PrimaryExtension;

	UE_LOG(LogTemp, Log, TEXT("Path: %s"), *Path);
	UBYGLocalizationStatics::SetLocalizationFromFile(Path);
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE( FBYGLocalizationEditorModule, BYGLocalizationEditor );


