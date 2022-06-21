// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BYGLocalizationStatics.generated.h"

class UStringTable;

UCLASS()
class BYGLOCALIZATION_API UBYGLocalizationStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Primary way for dynamically setting up localized strings. Uses the currently-loaded String Table
	UFUNCTION( BlueprintCallable, Category = "BYG|Localization" )
	static FText GetGameText( const FString& Key );

	// Returns false if either table or text does not exist
	UFUNCTION( BlueprintCallable, Category = "BYG|Localization" )
	static bool HasTextInTable( const FString& TableName, const FString& Key );

	// Use this to change the current localization, for example if the player changes their
	// preferred locale.
	// Need to specify a path and not just a locale code because fan-made localizations
	// can lead to multiple localizations of the same locale.
	UFUNCTION( BlueprintCallable, Category = "BYG|Localization" )
	static bool SetLocalizationFromFile(const FString& Path);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization|String Tables")
	static bool SetLocalizationFromAsset(UStringTable* StringTableAsset);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization|String Tables")
	static void PrintStringTableData(UStringTable* StringTableAsset);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization|String Tables", meta = (AutoCreateRefTerm = "StringTableID,Key"))
	static void SetTextAsStringTableEntry(UPARAM(ref) FText &Text, const FName &StringTableID, const FString &Key);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization|String Tables", meta = (DisplayName = "Import CSV to String Table"))
	static void ImportCSVToStringTable(FString CSVFilePath, UStringTable* StringTableAsset);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization|String Tables", meta = (DisplayName = "Export String Table to CSV"))
	static void ExportStringTableToCSV(UStringTable* StringTableAsset, FString CSVFilePath);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization|String Tables", meta = (DisplayName = "Import Default CSV to String Table"))
	static void ImportDefaultCSVToStringTable();

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization|String Tables", meta = (DisplayName = "Export Default String Table to CSV"))
	static void ExportDefaultStringTableToCSV();

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization")
	static void UpdateLocalizationTranslations();

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization")
	static void ReloadLocalization();

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization")
	static void AddNewEntryToTheLocalization(FString Key, FString SourceString, FString Comment);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization")
	static void UpdateCSV(FString Filename);

};
