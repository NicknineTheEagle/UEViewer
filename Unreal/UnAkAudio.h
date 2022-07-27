#ifndef __UNAKAUDIO_H__
#define __UNAKAUDIO_H__

#include "UE4Version.h"

#if UNREAL4

class UAkMediaAsset;

class UAkAssetData : public UObject
{
	DECLARE_CLASS(UAkAssetData, UObject);
public:
	uint32 CachedHash;
	FString BankLanguage;
	FByteBulkData Data;

	uint32 BankID;

	UAkAssetData() :
		BankLanguage("SFX"),
		BankID(0)
	{}

	BEGIN_PROP_TABLE
		PROP_UINT32(CachedHash)
		PROP_STRING(BankLanguage)
	END_PROP_TABLE

	void Serialize(FArchive& Ar)
	{
		guard(UAkAssetData::Serialize);

		Super::Serialize(Ar);
		Data.Serialize(Ar);

		unguard;
	}
};

class UAkAssetDataWithMedia : public UAkAssetData
{
	DECLARE_CLASS(UAkAssetDataWithMedia, UAkAssetData);
public:
	TArray<UAkMediaAsset*> MediaList;

	BEGIN_PROP_TABLE
		PROP_ARRAY(MediaList, PropType::UObject)
	END_PROP_TABLE
};

class UAkAssetPlatformData : public UObject
{
	DECLARE_CLASS(UAkAssetPlatformData, UObject);
public:
	//TMap<FString, UAkAssetData*> AssetDataPerPlatform;
	UAkAssetData* CurrentAssetData;

	//BEGIN_PROP_TABLE
	//	PROP_DROP(AssetDataPerPlatform) // editor only
	//	PROP_OBJ(CurrentAssetData) // transient
	//END_PROP_TABLE

	void Serialize(FArchive &Ar)
	{
		guard(UAkAssetPlatformData::Serialize);

		Super::Serialize(Ar);
		Ar << CurrentAssetData;

		unguard;
	}
};

class UAkAudioEventData : public UAkAssetDataWithMedia // UAkAssetDataSwitchContainer
{
	DECLARE_CLASS(UAkAudioEventData, UAkAssetDataWithMedia);
public:
	BEGIN_PROP_TABLE
		PROP_DROP(MaxAttenuationRadius)
		PROP_DROP(IsInfinite)
		PROP_DROP(MinimumDuration)
		PROP_DROP(MaximumDuration)
		PROP_DROP(LocalizedMedia) // DEPRECATED
		PROP_DROP(PostedEvents)
		PROP_DROP(UserDefinedSends)
		PROP_DROP(PostedTriggers)
		PROP_DROP(GroupValues)

		// UAkAssetDataSwitchContainer
		PROP_DROP(SwitchContainers)
		PROP_DROP(DefaultGroupValue)
		//PROP_DROP(AssetPlatform) // editor only
	END_PROP_TABLE

	void Serialize(FArchive &Ar)
	{
		guard(UAkAudioEventData::Serialize);

		Super::Serialize(Ar);
		BankID = *((uint32*)&(Data.BulkData[0x0c]));

		unguard;
	}
};

class UAkInitBankAssetData : public UAkAssetDataWithMedia
{
	DECLARE_CLASS(UAkInitBankAssetData, UAkAssetDataWithMedia);
public:
	BEGIN_PROP_TABLE
		PROP_DROP(PluginInfos)
	END_PROP_TABLE

	void Serialize(FArchive &Ar)
	{
		guard(UAkInitBankAssetData::Serialize);

		Super::Serialize(Ar);
		BankID = *((uint32*)&(Data.BulkData[0x0c]));

		unguard;
	}
};

class UAkAudioType : public UObject
{
	DECLARE_CLASS(UAkAudioType, UObject);
public:
	//FGuid ID;
	uint32 ShortID;
	TArray<UObject*> UserData;

	BEGIN_PROP_TABLE
		//PROP_DROP(ID) // editor only
		PROP_UINT32(ShortID)
		PROP_ARRAY(UserData, PropType::UObject)
	END_PROP_TABLE
};

class UAkAssetBase : public UAkAudioType
{
	DECLARE_CLASS(UAkAssetBase, UAkAudioType);
public:
	UAkAssetPlatformData* PlatformAssetData;

	UAkAssetBase() :
		PlatformAssetData(NULL)
	{}

	BEGIN_PROP_TABLE
		PROP_OBJ(PlatformAssetData)
	END_PROP_TABLE
};

// Probably obsolete in event-based layout.
class UAkAudioEvent : public UAkAssetBase
{
	DECLARE_CLASS(UAkAudioEvent, UAkAssetBase);
public:
	//TMap<FString, UAkAssetPlatformData*> LocalizedPlatformAssetDataMap;
	//AkAudioBank* RequiredBank;
	UAkAssetPlatformData* CurrentLocalizedPlatformData;

	UAkAudioEvent() :
		CurrentLocalizedPlatformData(NULL)
	{}

	BEGIN_PROP_TABLE
		PROP_DROP(LocalizedPlatformAssetDataMap) // Contains invalid export indexes???
		PROP_DROP(RequiredBank)
		//PROP_OBJ(CurrentLocalizedPlatformData) // transient
		PROP_DROP(MaxAttenuationRadius)
		PROP_DROP(IsInfinite)
		PROP_DROP(MinimumDuration)
		PROP_DROP(MaximumDuration)
	END_PROP_TABLE
};

class UAkInitBank : public UAkAssetBase
{
	DECLARE_CLASS(UAkInitBank, UAkAssetBase);
public:
	TArray<FString> AvailableAudioCultures;
	FString DefaultLanguage;

	BEGIN_PROP_TABLE
		PROP_ARRAY(AvailableAudioCultures, PropType::FString)
		PROP_STRING(DefaultLanguage)
	END_PROP_TABLE
};

struct FAkMediaDataChunk
{
	FByteBulkData Data;
	bool IsPrefetch;

	FAkMediaDataChunk() :
		IsPrefetch(false)
	{}

	friend FArchive& operator<<(FArchive& Ar, FAkMediaDataChunk& Chunk)
	{
		guard(FStreamedAudioChunk<<);

		Ar << Chunk.IsPrefetch;
		Chunk.Data.Serialize(Ar);

		return Ar;

		unguard;
	}
};

class UAkMediaAssetData : public UObject
{
	DECLARE_CLASS(UAkMediaAssetData, UObject);
public:
	bool IsStreamed;
	bool UseDeviceMemory;
	FString Language;
	FString AssetPlatform;

	TArray<FAkMediaDataChunk> DataChunks;

	UAkMediaAssetData() :
		IsStreamed(false),
		UseDeviceMemory(false),
		Language("SFX")
	{}

	BEGIN_PROP_TABLE
		PROP_BOOL(IsStreamed)
		PROP_BOOL(UseDeviceMemory)
		PROP_STRING(Language)
		//PROP_INT64(LastWriteTime) // editor only
		//PROP_UINT64(MediaContentHash) // editor only
		PROP_STRING(AssetPlatform)
	END_PROP_TABLE

	void Serialize(FArchive& Ar)
	{
		guard(UAkMediaAssetData::Serialize);

		Super::Serialize(Ar);

		int32 NumChunks = DataChunks.Num();
		Ar << NumChunks;
		if (Ar.IsLoading)
			DataChunks.AddDefaulted(NumChunks);

		for (int i = 0; i < NumChunks; i++)
		{
			guard(Chunk);
			Ar << DataChunks[i];
			unguardf("%d", i);
		}

		unguard;
	}
};

class UAkMediaAsset : public UObject
{
	DECLARE_CLASS(UAkMediaAsset, UObject);
public:
	uint32 Id;
	//TMap<FString, UAkMediaAssetData*> MediaAssetDataPerPlatform;
	FString MediaName;
	bool AutoLoad;
	TArray<UObject*> UserData;
	FString Language;
	UAkMediaAssetData* CurrentMediaAssetData;

	UAkMediaAsset() :
		AutoLoad(true),
		Language("SFX")
	{}

	BEGIN_PROP_TABLE
		PROP_UINT32(Id)
		//PROP_DROP(MediaAssetDataPerPlatform) // editor only
		PROP_STRING(MediaName)
		PROP_BOOL(AutoLoad)
		PROP_ARRAY(UserData, PropType::UObject)
		PROP_STRING(Language)
		//PROP_OBJ(CurrentMediaAssetData) // transient
	END_PROP_TABLE

	void Serialize(FArchive& Ar)
	{
		guard(UAkMediaAsset::Serialize);

		Super::Serialize(Ar);
		Ar << CurrentMediaAssetData;

		unguard;
	}
};

#endif // UNREAL4

#define REGISTER_AKSOUND_CLASSES			\
	REGISTER_CLASS(UAkMediaAsset)			\
	REGISTER_CLASS(UAkMediaAssetData)		\
	REGISTER_CLASS(UAkAudioEvent)			\
	REGISTER_CLASS(UAkAudioEventData)		\
	REGISTER_CLASS(UAkAssetPlatformData)	\
	REGISTER_CLASS(UAkInitBank)				\
	REGISTER_CLASS(UAkInitBankAssetData)	\
	REGISTER_CLASS_ALIAS(UAkMediaAsset, UAkLocalizedMediaAsset)	\
	REGISTER_CLASS_ALIAS(UAkMediaAsset, UAkExternalMediaAsset)

#endif // __UNAKAUDIO_H__
