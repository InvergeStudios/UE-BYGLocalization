// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationStatics.h"
#include "BYGLocalizationCoreMinimal.h"
#include "BYGLocalizationSettings.h"
#include "BYGLocalizationModule.h"
#include "BYGLocalization.h"

#include <Internationalization/StringTableCore.h>
#include <Internationalization/StringTableRegistry.h>
#include <Internationalization/StringTable.h>
#include <DesktopPlatform/Public/DesktopPlatformModule.h>
#include <EditorDirectories.h>

bool UBYGLocalizationStatics::HasTextInTable( const FString& TableName, const FString& Key )
{
	FStringTableConstPtr StringTable = FStringTableRegistry::Get().FindStringTable( *TableName );

	if ( StringTable.IsValid() )
	{
		FStringTableEntryConstPtr Entry = StringTable->FindEntry( *Key );
		return Entry.IsValid();
	}
	return false;
}

bool GetTextFromTable( const FString& TableName, const FString& Key, FText& FoundText )
{
	// Not using UE4's default method because it doesn't differentiate between missing a table and
	// missing a key
	FStringTableConstPtr StringTable = FStringTableRegistry::Get().FindStringTable( *TableName );

	if ( StringTable.IsValid() )
	{
		FStringTableEntryConstPtr pEntry = StringTable->FindEntry( *Key );
		if ( pEntry.IsValid() )
		{
			FTextDisplayStringPtr pStr = pEntry->GetDisplayString();
			FoundText = FText::FromString( *pStr );
			return true;
		}
		else
		{
			UE_LOG( LogBYGLocalization, Error, TEXT( "Could not find key '%s' in string table '%s'" ), *TableName, *Key );
		}
	}
	else
	{
		UE_LOG( LogBYGLocalization, Error, TEXT( "Could not find string table '%s'" ), *TableName, *Key );
	}

	// Show compact error message: "key not found" or "table not found" + the id
	FoundText = FText::FromString( FString::Printf( TEXT( "(%s:%s)" ),
		StringTable.IsValid() ? *FString( TEXT( "KNF" ) ) : *FString( TEXT( "TNF" ) ),
		StringTable.IsValid() ? *Key : *TableName ) );

	return false;
}

FText UBYGLocalizationStatics::GetGameText( const FString& Key )
{
	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	FText Result;
	bool bFound = GetTextFromTable( Settings->StringtableID, Key, Result );
	if ( !bFound )
	{
		// Fall back to English if we're not using English and we didn't get the key in the non-English locale  
		bFound = GetTextFromTable( Settings->PrimaryLanguageCode, Key, Result );
	}
	return Result;
}

bool UBYGLocalizationStatics::SetLocalizationByCode(const FString& Code)
{
	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	if (Code != Settings->PrimaryLanguageCode && !Settings->LanguageCodesInUse.Contains(Code) && Code != "Debug")
		return false;
	
	FBYGLocalizationModule::Get().SetCurrentLanguageCode(Code);

	const TArray<FBYGLocaleInfo> Localizations = FBYGLocalizationModule::Get().GetLocalization()->GetAvailableLocalizations();

	TArray<FString> Categories = Settings->LocalizationCategories;
	Categories.AddUnique("Game");

	for (const FString Category : Categories)
	{
		FStringTableRegistry::Get().UnregisterStringTable(FName(*Category));

		for (const FBYGLocaleInfo Localization : Localizations)
		{
			if (Localization.Category.ToString() == Category && Localization.LocaleCode == Code)
			{
				FStringTableRegistry::Get().Internal_LocTableFromFile(
					FName(*Category),
					Category,
					Localization.FilePath,
					FPaths::ProjectContentDir()
				);

				break;
			}
		}
	}

#if !WITH_EDITOR
	FString UE_Code = (Code == "Debug") ? "en" : Code;
	FInternationalization::Get().SetCurrentCulture(UE_Code);
	FInternationalization::Get().SetCurrentLanguageAndLocale(UE_Code);
#endif

	return true;
}

void UBYGLocalizationStatics::SetTextAsStringTableEntry(FText &Text, const FName &StringTableID, const FString &Key)
{
	//FText::FText(FName InTableId, FString InKey, const EStringTableLoadingPolicy InLoadingPolicy)
	Text = FText::FromStringTable(StringTableID, Key, EStringTableLoadingPolicy::FindOrLoad);
}

void UBYGLocalizationStatics::UpdateLocalizationTranslations()
{
	FBYGLocalizationModule::Get().UpdateTranslations();
}

void UBYGLocalizationStatics::ReloadLocalization()
{
	FBYGLocalizationModule::Get().ReloadLocalizations();
}

void UBYGLocalizationStatics::AddNewEntryToTheLocalization(const FString &Category, const FString &Key, const FString &SourceString, const FString &Comment)
{
	if (Category.IsEmpty() || Key.IsEmpty() || SourceString.IsEmpty())
		return;
	
	FStringTablePtr StringTable = FStringTableRegistry::Get().FindMutableStringTable(FName(*Category));
	if (StringTable.IsValid())
	{
		StringTable->SetSourceString(Key, SourceString);
		StringTable->SetMetaData(Key, TEXT("Comment"), Comment);
		StringTable->SetMetaData(Key, TEXT("Primary"), SourceString);
		StringTable->SetMetaData(Key, TEXT("Status"), FString("New Key Added from UE4"));
	}
}

void UBYGLocalizationStatics::UpdateCSV(const FString &Categroy, const FString &Filename)
{
	if (Categroy.IsEmpty() || Filename.IsEmpty())
		return;

	FStringTablePtr StringTable = FStringTableRegistry::Get().FindMutableStringTable(FName(*Categroy));

	if (StringTable.IsValid())
	{
		StringTable->ExportStrings(Filename);
	}
}

void UBYGLocalizationStatics::GetLocalizationFilePath(const FString &LanguageCode, const FString &Categroy, FString &FilePath)
{
	const TArray<FBYGLocaleInfo> Localizations = FBYGLocalizationModule::Get().GetLocalization()->GetAvailableLocalizations();
	for (const FBYGLocaleInfo &Localization : Localizations)
	{
		if (Localization.LocaleCode == LanguageCode && Localization.Category.ToString() == Categroy)
		{
			FilePath = Localization.FilePath;
			return;
		}
	}
}

void UBYGLocalizationStatics::GetLocalizationLanguageCodes(TArray<FString> &LanguageCodes)
{
	LanguageCodes.Reset();

	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();
	LanguageCodes.Add(Settings->PrimaryLanguageCode);

	for (const FString &LanguageCode : Settings->LanguageCodesInUse)
	{
		LanguageCodes.AddUnique(LanguageCode);
	}
}

void UBYGLocalizationStatics::GetLocalizationCategories(TArray<FString> &Categories)
{
	Categories.Reset();

	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();
	FString PrimaryCode = Settings->PrimaryLanguageCode;
	const TArray<FBYGLocaleInfo> Localizations = FBYGLocalizationModule::Get().GetLocalization()->GetAvailableLocalizations();
	for (const FBYGLocaleInfo &Localization : Localizations)
	{
		if(Localization.LocaleCode == PrimaryCode)
			Categories.AddUnique(Localization.Category.ToString());
	}
}

FString UBYGLocalizationStatics::GetCurrentLanguageCode()
{
	return FBYGLocalizationModule::Get().GetCurrentLanguageCode();
}

