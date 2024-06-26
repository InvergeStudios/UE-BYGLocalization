// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Internationalization/Culture.h"
#include "Delegates/DelegateCombinations.h"
#include "BYGLocalization.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE(FOnLocalizationChangedCallback);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLocalizationChanged);


enum class EBYGLocEntryStatus : uint8
{
	None,
	New,
	Modified,
	Deprecated
};

enum class EBYGLocErrorFlags : uint8
{
	Success,
	InvalidHeader,
};

struct FBYGLocaleInfo
{
	FString LocaleCode;
	FText LocalizedName;
	FString Category;
	FString FilePath;
};

struct FBYGLocalizationEntry
{
	FBYGLocalizationEntry() {}
	FBYGLocalizationEntry( FString Key_, FString Translation_, FString Comment_ )
		: Key( Key_ )
		, Translation( Translation_ )
		, Comment( Comment_ )
	{
	}
	FString Key;
	FString Translation;
	FString Comment;
	FString Primary;
	FString OldPrimary; // Not in CSV
	EBYGLocEntryStatus Status = EBYGLocEntryStatus::None;
};

// Internal data structure for 
struct FBYGLocaleData
{
public:
	FBYGLocaleData() {}
	FBYGLocaleData( const TArray<FBYGLocalizationEntry>& NewEntries );

	inline const TArray<FBYGLocalizationEntry>* GetEntriesInOrder() const { return &EntriesInOrder; }
	inline const TMap<FString, int32>* GetKeyToIndex() const { return &KeyToIndex; }

protected:
	TArray<FBYGLocalizationEntry> EntriesInOrder;
	TMap<FString, int32> KeyToIndex;
};

typedef TMap<EBYGLocEntryStatus, int32> BYGLocStats;

// Internal data structure used for	updating non-primary localizations based on the information in the primary
// We re-order entries in the non-primary to match those of the 
class BYGLOCALIZATION_API UBYGLocalization
{
public:

	// Returns a map from filename to display name
	TArray<FBYGLocaleInfo> GetAvailableLocalizations(TOptional<FString> LocaleFilter = TOptional<FString>(), TOptional<FString> CategoryFilter = TOptional<FString>()) const;

	// Returns false when no primary translations found
	bool UpdateTranslations();

	bool GetLocalizationStats( const FString& Filename, BYGLocStats& StatusCounts ) const;

	bool GetLocaleFromPreferences( FBYGLocaleInfo& FoundLocale ) const;

	FBYGLocaleInfo GetCultureFromFilename( const FString& FileWithPath ) const;

	// Returns an expected filename based on the user settings 
	FString GetFilenameFromLanguageCode(const FString& LanguageCode, const FString& Category) const;

	FString GetFileWithPathFromLanguageCode(const FString& LanguageCode, const FString& Category) const;

	bool GetAuthorForLocale( const FString& Filename, FText& Author ) const;

	void BindOnLocalizationChanged(const FOnLocalizationChangedCallback& Callback);
	void UnbindOnLocalizationChanged(const FOnLocalizationChangedCallback& Callback);
	void UnbindObjectFromOnLocalizationChanged(UObject* ObjectToUnbind);

	void CallOnLocalizationChanged();

public:

	FOnLocalizationChanged OnLocalizationChanged;

protected:

	bool GetLocalizationDataFromFile( const FString& Filename, FBYGLocaleData& LocalizationData ) const;
	bool UpdateTranslationFile(const FString& Path, const TArray<FBYGLocalizationEntry>* PrimaryEntriesInOrder, const TMap<FString, int32>* PrimaryKeyToIndex);
	bool UpdateDebugFile(const FString& Path, const TArray<FBYGLocalizationEntry>* PrimaryEntriesInOrder, const TMap<FString, int32>* PrimaryKeyToIndex);
	void GenerateDebugTranslation(const FString& PrimaryEntry, FString &DebugTranslation);

	TArray<FString> GetAllLocalizationFiles() const;
	// Writes datastructure to CSV but with explicit quoting etc.
	bool WriteCSV( const TArray<FBYGLocalizationEntry>& Entries, const FString& Filename );

	FString RemovePrefixSuffix(const FString& FileWithExtension) const;
	void SplitCategoryAndCulture(const FString& CategoryAndCulture, FString &Category, FString &Culture) const;

	static FString ReplaceCharWithEscapedChar( const FString& Str );

	static FString LazyWrap( const FString& InStr, bool bForceWrap = false );

	// Hacky testing
	friend class FBYGEscapeCharacterTest;
	friend class FBYGLazyWrapTest;
	friend class FBYGWriteCSVTest;
	friend class FBYGFullLoopTest;

};

