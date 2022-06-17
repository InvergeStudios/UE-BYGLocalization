// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationStatics.h"
#include "BYGLocalizationCoreMinimal.h"
#include "BYGLocalizationSettings.h"
#include "BYGLocalizationModule.h"
#include "BYGLocalization.h"

#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"
#include "Internationalization/StringTable.h"

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

bool UBYGLocalizationStatics::SetLocalizationFromFile( const FString& Path )
{
	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	FStringTableRegistry::Get().UnregisterStringTable( FName( *Settings->StringtableID ) );
	FStringTableRegistry::Get().Internal_LocTableFromFile(
		FName( *Settings->StringtableID ),
		Settings->StringtableNamespace,
		Path,
		FPaths::ProjectContentDir() 
	);

#if !WITH_EDITOR
	// Only use UE4's locale changing system outside of the editor, or stuff gets weird
	const FBYGLocaleInfo Basic = FBYGLocalizationModule::Get().GetLocalization()->GetCultureFromFilename( Path );
	FInternationalization::Get().SetCurrentCulture( Basic.LocaleCode );
	FInternationalization::Get().SetCurrentLanguageAndLocale( Basic.LocaleCode );
#endif

	return true;
}

bool UBYGLocalizationStatics::SetLocalizationFromAsset(UStringTable* StringTableAsset)
{
	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	FStringTableRegistry::Get().UnregisterStringTable(FName(*Settings->StringtableID));
	FStringTableRegistry::Get().RegisterStringTable(FName(*Settings->StringtableID), StringTableAsset->GetMutableStringTable());

#if !WITH_EDITOR
	// Only use UE4's locale changing system outside of the editor, or stuff gets weird
	const FBYGLocaleInfo Basic = FBYGLocalizationModule::Get().GetLocalization()->GetCultureFromFilename(StringTableAsset->GetName());
	FInternationalization::Get().SetCurrentCulture(Basic.LocaleCode);
	FInternationalization::Get().SetCurrentLanguageAndLocale(Basic.LocaleCode);
#endif

	return true;
}

void UBYGLocalizationStatics::PrintStringTableData(UStringTable* StringTableAsset)
{
	if (!StringTableAsset)
		return;

	FStringTableConstRef StringTable = StringTableAsset->GetStringTable();

	const FString SourceLocation = StringTableAsset->GetPathName();

	StringTable->EnumerateSourceStrings([&](const FString& InKey, const FString& InSourceString) -> bool
		{
			UE_LOG(LogTemp, Log, TEXT("Key: %s -> SounceString: %s"), *InKey, *InSourceString);
			
			if (!InSourceString.IsEmpty())
			{
// 				FGatherableTextData& GatherableTextData = FindOrAddTextData(InSourceString);
// 
// 				FTextSourceSiteContext& SourceSiteContext = GatherableTextData.SourceSiteContexts[GatherableTextData.SourceSiteContexts.AddDefaulted()];
// 				SourceSiteContext.KeyName = InKey;
// 				SourceSiteContext.SiteDescription = SourceLocation;
// 				SourceSiteContext.IsEditorOnly = false;
// 				SourceSiteContext.IsOptional = false;

				StringTable->EnumerateMetaData(InKey, [&](const FName InMetaDataId, const FString& InMetaData)
					{
						UE_LOG(LogTemp, Log, TEXT("MetaDataId: %s -> MetaData: %s"), *InMetaDataId.ToString(), *InMetaData);
						//SourceSiteContext.InfoMetaData.SetStringField(InMetaDataId.ToString(), InMetaData);
						return true; // continue enumeration
					});
			}

			return true; // continue enumeration
		});
}

void UBYGLocalizationStatics::SetTextAsStringTableEntry(FText &Text, const FName &StringTableID, const FString &Key)
{
	//FText::FText(FName InTableId, FString InKey, const EStringTableLoadingPolicy InLoadingPolicy)
	Text = FText::FromStringTable(StringTableID, Key, EStringTableLoadingPolicy::FindOrLoad);
}

