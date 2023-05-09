// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationStatics.h"
#include "BYGLocalizationCoreMinimal.h"
#include "BYGLocalizationSettings.h"
#include "BYGLocalizationModule.h"
#include "BYGLocalization.h"

#include <Internationalization/StringTableCore.h>
#include <Internationalization/StringTableRegistry.h>
#include <Internationalization/StringTable.h>

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
			FTextConstDisplayStringPtr pStr = pEntry->GetDisplayString();
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
			if (Localization.Category == Category && Localization.LocaleCode == Code)
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
	
	const FStringTablePtr StringTable = FStringTableRegistry::Get().FindMutableStringTable(FName(*Category));
	if (StringTable.IsValid())
	{
		StringTable->SetSourceString(Key, SourceString);
		StringTable->SetMetaData(Key, TEXT("Comment"), Comment);
		StringTable->SetMetaData(Key, TEXT("Primary"), SourceString);
		StringTable->SetMetaData(Key, TEXT("Status"), FString("New Key Added from UE4"));
	}
}

void UBYGLocalizationStatics::RemoveEntryFromLocalization(const FString& Category, const FString& Key)
{
	if (Category.IsEmpty() || Key.IsEmpty())
		return;
	
	const FStringTablePtr StringTable = FStringTableRegistry::Get().FindMutableStringTable(FName(*Category));
	if (StringTable.IsValid())
	{
		StringTable->RemoveSourceString(Key);
	}
}

bool UBYGLocalizationStatics::UpdateLocalizationSourceString(const FString& Category, const FString& Key, const FString& SourceString)
{
	if (Category.IsEmpty() || Key.IsEmpty() || SourceString.IsEmpty())
		return false;

	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();
	const FString MainLanguage = Settings->PrimaryLanguageCode;
	const FString CurrentLanguageCode = FBYGLocalizationModule::Get().GetCurrentLanguageCode();
	const bool InMainLanguage = MainLanguage == CurrentLanguageCode;
	
	const FStringTablePtr StringTable = FStringTableRegistry::Get().FindMutableStringTable(FName(*Category));
	if (StringTable.IsValid())
	{
		FString OldSourceString;
		StringTable->GetSourceString(Key, OldSourceString);

		if(OldSourceString == SourceString)
			return false;
		
		StringTable->SetSourceString(Key, SourceString);

		if(InMainLanguage)
		{
			StringTable->SetMetaData(Key, TEXT("Primary"), SourceString);
			const FString NewStatus = "Changed SourceString from [" + OldSourceString + "] to [" + SourceString + "]";
			StringTable->SetMetaData(Key, TEXT("Status"), NewStatus);			
		}
		else
		{
			StringTable->SetMetaData(Key, TEXT("Status"), FString(""));		
		}
		
		return true;
	}

	return false;
}

void UBYGLocalizationStatics::UpdateLocalizationEntryMetadata(const FString& Category, const FString& Key,	const FName &Metadata, const FString &Value)
{
	if (Category.IsEmpty() || Key.IsEmpty() ||
		(!Metadata.IsEqual(TEXT("Comment")) && !Metadata.IsEqual(TEXT("Primary")) && !Metadata.IsEqual(TEXT("Status")))
		)
		return;
	
	const FStringTablePtr StringTable = FStringTableRegistry::Get().FindMutableStringTable(FName(*Category));
	if (StringTable.IsValid())
	{
		StringTable->SetMetaData(Key, Metadata, Value);
	}
}

void UBYGLocalizationStatics::UpdateCSV(const FString &Category, const FString &Filename)
{
	if (Category.IsEmpty() || Filename.IsEmpty())
		return;

	ExportStrings(FName(*Category), Filename);
}

bool UBYGLocalizationStatics::ExportStrings(const FName StringTableName, const FString& InFilename)
{
	const FStringTablePtr StringTable = FStringTableRegistry::Get().FindMutableStringTable(StringTableName);

	if (!StringTable.IsValid())
	{
		return false;
	}

	FString ExportedStrings;
	TArray<FName> MetaDataColumnNames;
	MetaDataColumnNames.Add(TEXT("Comment"));
	MetaDataColumnNames.Add(TEXT("Primary"));
	MetaDataColumnNames.Add(TEXT("Status"));

	{
		// Write header
		ExportedStrings += TEXT("Key,SourceString,Comment,Primary,Status\r\n");

		// Write entries
		TArray<FString> KeysFromStringTable;
		StringTable->EnumerateSourceStrings([&](const FString& InKey, const FString& InSourceString) -> bool
		{			
			FString ExportedKey = InKey.ReplaceCharWithEscapedChar();
			ExportedKey.ReplaceInline(TEXT("\""), TEXT("\"\""));

			FString ExportedSourceString = InSourceString.ReplaceCharWithEscapedChar();
			ExportedSourceString.ReplaceInline(TEXT("\""), TEXT("\"\""));

			ExportedStrings += TEXT("\"");
			ExportedStrings += ExportedKey;
			ExportedStrings += TEXT("\"");

			ExportedStrings += TEXT(",");

			ExportedStrings += TEXT("\"");
			ExportedStrings += ExportedSourceString;
			ExportedStrings += TEXT("\"");

			for (const FName& MetaDataColumnName : MetaDataColumnNames)
			{
				FString ExportedMetaData = StringTable->GetMetaData(InKey, MetaDataColumnName).ReplaceCharWithEscapedChar();
				ExportedMetaData.ReplaceInline(TEXT("\""), TEXT("\"\""));

				ExportedStrings += TEXT(",");

				ExportedStrings += TEXT("\"");
				ExportedStrings += ExportedMetaData;
				ExportedStrings += TEXT("\"");
			}

			ExportedStrings += TEXT("\r\n");
			KeysFromStringTable.Add(InKey);
			return true; // continue enumeration
		});
	}

	return FFileHelper::SaveStringToFile(ExportedStrings, *InFilename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

void UBYGLocalizationStatics::GetLocalizationFilePath(const FString &LanguageCode, const FString &Category, FString &FilePath)
{
	const TArray<FBYGLocaleInfo> Localizations = FBYGLocalizationModule::Get().GetLocalization()->GetAvailableLocalizations();
	for (const FBYGLocaleInfo &Localization : Localizations)
	{
		if (Localization.LocaleCode == LanguageCode && Localization.Category == Category)
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
			Categories.AddUnique(Localization.Category);
	}
}

bool UBYGLocalizationStatics::AddNewCategory(FString CategoryToAdd /*= "NewCategory"*/)
{
	UBYGLocalizationSettings* Settings = GetMutableDefault<UBYGLocalizationSettings>();
	
	if(CategoryToAdd.Contains(TEXT(" ")))
		return false;

	if (CategoryToAdd != "" && !Settings->LocalizationCategories.Contains(CategoryToAdd))
	{
		Settings->LocalizationCategories.AddUnique(CategoryToAdd);
		Settings->TryUpdateDefaultConfigFile();


		FString LocalizationDirPath = Settings->PrimaryLocalizationDirectory.Path.Replace(TEXT("/Game"), *FPaths::ProjectContentDir());
		FString FullPath = FString::Printf(TEXT("%s/%s/%s"),
				*LocalizationDirPath,
				*Settings->PrimaryLanguageCode,
				*FBYGLocalizationModule::Get().GetLocalization()->GetFilenameFromLanguageCode(Settings->PrimaryLanguageCode, CategoryToAdd));

		FPaths::RemoveDuplicateSlashes(FullPath);
		FString ExportedStrings = "Key,SourceString,Comment,Primary,Status\r\n";

		UE_LOG(LogBYGLocalization, Log, TEXT("AddNewCategory: %s"), *FullPath);
		if (FFileHelper::SaveStringToFile(ExportedStrings, *FullPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			UpdateLocalizationTranslations();
		}

		return true;
	}


	return false;
}

bool UBYGLocalizationStatics::AddNewLanguage(FString NewLanguage /*= "LanguageCode"*/)
{
	UBYGLocalizationSettings* Settings = GetMutableDefault<UBYGLocalizationSettings>();

	if (NewLanguage.Contains(TEXT(" ")))
		return false;

	if (NewLanguage != "" && Settings->PrimaryLanguageCode != NewLanguage && !Settings->LanguageCodesInUse.Contains(NewLanguage))
	{
		Settings->LanguageCodesInUse.AddUnique(NewLanguage);
		Settings->TryUpdateDefaultConfigFile();
		UpdateLocalizationTranslations();
		return true;
	}


	return false;
}

FString UBYGLocalizationStatics::GetCurrentLanguageCode()
{
	return FBYGLocalizationModule::Get().GetCurrentLanguageCode();
}

