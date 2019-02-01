// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#if PLATFORM_WINDOWS
#include "AllowWindowsPlatformTypes.h" 
#include "Windows.h"
#include "winspool.h"
#include "iostream"
#include "time.h"
#include "fstream"
#include "string"
#include "HideWindowsPlatformTypes.h"
#endif
#include "Engine.h"
#include "ImageUtils.h"

//opencv
//#include "opencv/cv.hpp"

#include "JPrinterBPLibrary.generated.h"

//using namespace cv;

#if PLATFORM_WINDOWS
struct printerInfo
{
	LPCWSTR portName;
	LPCWSTR driver;
	LPCWSTR deviceName;
	LPCWSTR output = NULL;
};
#define SRCCOPY2 (unsigned long)0x00CC0020
#endif

UENUM(BlueprintType)
enum class EPaperSize : uint8
{
	None	UMETA(DisplayName = "None"),
	letter 	UMETA(DisplayName = "Letter 8 1/2 x 11 in"),
	photo 	UMETA(DisplayName = "Photo 4 x 6 in"),
};

UCLASS()
class UJPrinterBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintPure, Category = JPrinter)
		static TArray<FString> getPrinterList();

	UFUNCTION(BlueprintPure, Category = JPrinter)
		static FString getPrimaryPrinterName();

	UFUNCTION(BlueprintCallable, Category = JPrinter)
		static bool printImage(FString path, FString printer, bool usePrimary = true);

	UFUNCTION(BlueprintCallable, Category = JPrinter)
		static bool printTexture2D(UTexture2D* texture, FString printer, bool usePrimary, EPaperSize paperSize = EPaperSize::None);


	static FString Replace(FString source, FString in, FString out);

#if PLATFORM_WINDOWS
	static printerInfo getPrinterInfo(FString printer, bool usePrimary);
	static HBITMAP getHBITMAPFromData(TArray<FColor>& bgraData, int32 width, int32 height, int32 bitDepth);
#endif

};
