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
	UFUNCTION(BlueprintCallable, Category = "BYG|Localization")
	static bool SetLocalizationByCode(const FString& Code);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization|String Tables", meta = (AutoCreateRefTerm = "StringTableID,Key"))
	static void SetTextAsStringTableEntry(UPARAM(ref) FText &Text, const FName &StringTableID, const FString &Key);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization")
	static void UpdateLocalizationTranslations();

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization")
	static void ReloadLocalization();

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization", meta = (AutoCreateRefTerm = "Category,Key,SourceString,Comment"))
	static void AddNewEntryToTheLocalization(const FString &Category, const FString &Key, const FString &SourceString, const FString &Comment);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization", meta = (AutoCreateRefTerm = "Category,Key,SourceString,Comment"))
	static void RemoveEntryFromLocalization(const FString &Category, const FString &Key);
	
	UFUNCTION(BlueprintCallable, Category = "BYG|Localization", meta = (AutoCreateRefTerm = "Category,Key,SourceString"))
	static bool UpdateLocalizationSourceString(const FString &Category, const FString &Key, const FString &SourceString);
	
	UFUNCTION(BlueprintCallable, Category = "BYG|Localization", meta = (AutoCreateRefTerm = "Category,Key,Metadata,Value"))
	static void UpdateLocalizationEntryMetadata(const FString &Category, const FString &Key, const FName &Metadata, const FString &Value);
	
	UFUNCTION(BlueprintCallable, Category = "BYG|Localization", meta = (AutoCreateRefTerm = "Category,Filename"))
	static void UpdateCSV(const FString &Category, const FString &Filename);

	static bool ExportStrings(const FName StringTableName, const FString& InFilename);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization", meta = (AutoCreateRefTerm = "LanguageCode,Categroy"))
	static void GetLocalizationFilePath(const FString &LanguageCode, const FString &Category, FString &FilePath);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization")
	static void GetLocalizationLanguageCodes(TArray<FString> &LanguageCodes);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localization")
	static void GetLocalizationCategories(TArray<FString> &Categories);

	UFUNCTION(BlueprintCallable, Category = "BYG|Localizaton", meta = (ReturnDisplayName = "Category Added"))
	static bool AddNewCategory(FString CategoryToAdd = "NewCategory");

	UFUNCTION(BlueprintCallable, Category = "BYG|Localizaton", meta = (ReturnDisplayName = "Language Added"))
	static bool AddNewLanguage(FString NewLanguage = "LanguageCode");

	UFUNCTION(BlueprintPure, Category = "BYG|Localization")
	static FString GetCurrentLanguageCode();

};
