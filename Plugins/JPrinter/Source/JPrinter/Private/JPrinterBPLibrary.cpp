// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "JPrinterBPLibrary.h"

#define print(txt) GEngine->AddOnScreenDebugMessage(-1,10,FColor::Green, txt)
DEFINE_LOG_CATEGORY_STATIC(JPrinterLog, Log, All);

UJPrinterBPLibrary::UJPrinterBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

TArray<FString> UJPrinterBPLibrary::getPrinterList()
{
	TArray<FString> out;
#if PLATFORM_WINDOWS
	LPBYTE pPrinterEnum;
	unsigned long pcbNeeded, pcbReturned;
	PRINTER_INFO_2* printerInfo = NULL;
	EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, NULL, 0, &pcbNeeded, &pcbReturned);

	pPrinterEnum = new BYTE[pcbNeeded];
	if (EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, pPrinterEnum, pcbNeeded, &pcbNeeded, &pcbReturned))
	{
		printerInfo = ((PRINTER_INFO_2*)pPrinterEnum);
		for (unsigned short i = 0; i < pcbReturned; i++)
		{
			TCHAR* wname = printerInfo[i].pPrinterName;
			FString name = wname;
			out.Add(name);
		}
	}
#endif
	return out;
}

FString UJPrinterBPLibrary::getPrimaryPrinterName()
{
	FString out;
#if PLATFORM_WINDOWS
	unsigned long size = 0;
	GetDefaultPrinter(NULL, &size);
	if (size)
	{
		TCHAR* buffer = new TCHAR[size];
		GetDefaultPrinter(buffer, &size);
		out = buffer;
	}
#endif
	return out;
}

#if PLATFORM_WINDOWS
printerInfo UJPrinterBPLibrary::getPrinterInfo(FString printer, bool usePrimary)
{
	printerInfo out;
	unsigned long size = 0;
	LPWSTR defaultName = NULL;
	GetDefaultPrinter(NULL, &size);
	if (size)
	{
		TCHAR* buffer = new TCHAR[size];
		GetDefaultPrinter(buffer, &size);
		defaultName = buffer;
	}

	if (defaultName)
	{
		out.deviceName = defaultName;
		LPBYTE pPrinterEnum;
		unsigned long pcbNeeded, pcbReturned;
		PRINTER_INFO_2* printerInfo = NULL;
		EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, NULL, 0, &pcbNeeded, &pcbReturned);

		pPrinterEnum = new BYTE[pcbNeeded];
		if (EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, pPrinterEnum, pcbNeeded, &pcbNeeded, &pcbReturned))
		{
			printerInfo = ((PRINTER_INFO_2*)pPrinterEnum);
			std::wstring wsdefaultName(defaultName);
			std::string sdefaultName(wsdefaultName.begin(), wsdefaultName.end());

			for (unsigned int i = 0; i < pcbReturned; i++)
			{
				LPWSTR name = printerInfo[i].pPrinterName;
				std::wstring wsname(name);
				std::string sname(wsname.begin(), wsname.end());
				if (usePrimary)
				{
					if (sname == sdefaultName)
					{
						out.driver = printerInfo[i].pDriverName;
						out.portName = printerInfo[i].pPortName;
					}
				}
				else
				{
					std::string choosen(TCHAR_TO_ANSI(*printer));
					if (sname == choosen)
					{
						out.driver = printerInfo[i].pDriverName;
						out.portName = printerInfo[i].pPortName;
					}
				}
				
			}
		}
	}
	return out;
}

HBITMAP UJPrinterBPLibrary::getHBITMAPFromData(TArray<FColor>& bgraData, int32 width, int32 height, int32 bitDepth)
{
	if (bgraData.Num() <= 0) return NULL;

	uint8* rgb = new uint8[width * height * 3];
	for (int i = 0; i < (width * height); i++)
	{
		rgb[3 * i + 0] = bgraData[i].B;
		rgb[3 * i + 1] = bgraData[i].G;
		rgb[3 * i + 2] = bgraData[i].R;
	}
	// Create DIB
	HBITMAP hbmp = CreateBitmap(width, height, 1, 24, rgb);
	if (hbmp == NULL) {
		delete rgb;
		return hbmp;
	}
	delete rgb;
	return hbmp;
}
#endif

FString UJPrinterBPLibrary::Replace(FString source, FString in, FString out)
{
	TArray<TCHAR> ar = source.GetCharArray();
	FString result;
	for (int i = 0; i < ar.Num(); i++)
	{
		if (ar[i] != in[0])
		{
			result += ar[i];
		}
		else
		{
			result += out;
		}
	}
	return result;
}

bool UJPrinterBPLibrary::printImage(FString path, FString printer, bool usePrimary)
{
	if (!FPaths::FileExists(path)) return false;

	if (usePrimary)
	{
		printer = getPrimaryPrinterName();
	}

	path = "\"" + path + "\"";
	path = Replace(path, "/", "\\");
	FString fullPath = "rundll32 c:\\WINDOWS\\system32\\shimgvw.dll,ImageView_PrintTo /pt " + path + " \"" + printer + "\"";

	UE_LOG(JPrinterLog, Log, TEXT("%s"), *fullPath);
	int error = WinExec(TCHAR_TO_ANSI(*fullPath), SW_HIDE);
	return (error == 33) ? true : false;

}

bool UJPrinterBPLibrary::printTexture2D(UTexture2D* texture, FString printer, bool usePrimary, EPaperSize paperSize)
{
	if (!texture) return false;

	//const double DPIConvertion = 142.0643729189789;
	//const float DPIConvertion = 142.0643729189789f;
	const float DPIConvertion = 142.06437f;

	bool out = false;
	int32 width = texture->GetSizeX();
	int32 height = texture->GetSizeY();
	int32 bitDepth = 24;
	TArray<FColor> colorData;
	colorData.Init(FColor(), width * height);
	FTexture2DMipMap& Mip = texture->PlatformData->Mips[0];
	uint8* Data = (uint8*)Mip.BulkData.Lock(LOCK_READ_WRITE);

	for (int i = 0; i < colorData.Num(); i++)
	{
		colorData[i].R = Data[4 * i + 2];
		colorData[i].G = Data[4 * i + 1];
		colorData[i].B = Data[4 * i + 0];
		colorData[i].A = 255;
	}

	Mip.BulkData.Unlock();
	texture->UpdateResource();

	if (paperSize != EPaperSize::None)
	{
		
		float scale = 1;
		switch (paperSize)
		{
		case EPaperSize::letter:
			if (width == 2969) break;
			scale = FMath::FloorToFloat((2969.f / width) * 100) / 100.f;
			break;
		case EPaperSize::photo:
			if (width == 1373) break;
			scale = FMath::FloorToFloat((1373.f / width) * 1000) / 1000.f;
			break;
		default:
			break;
		}

		if (scale != 1)
		{
			TArray<FColor> fixColorData;
			FImageUtils::ImageResize(width, height, colorData, width * scale, height * scale, fixColorData, true);
			width = width * scale;
			height = height * scale;
			colorData = fixColorData;
		}
		
	}
	
#if PLATFORM_WINDOWS
	HWND currentWindow = GetActiveWindow();
	printerInfo dev = getPrinterInfo(printer, usePrimary);
	
	HBITMAP hBMP = getHBITMAPFromData(colorData, width, height, bitDepth);
	//HBITMAP hBMP = (HBITMAP)LoadImage(NULL, L"D:/gokuHQ.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	HDC printerHDC = CreateDC(dev.driver, dev.deviceName, dev.portName, NULL);
	if (printerHDC)
	{
		HDC hdc = CreateCompatibleDC(printerHDC);
		SelectObject(hdc, hBMP);
		Escape(printerHDC, STARTDOC, 8, "Happy-Doc", NULL);
		BitBlt(printerHDC, 0, 0, width, height, hdc, 0, 0, SRCCOPY2);
		Escape(printerHDC, NEWFRAME, 0, NULL, NULL);
		Escape(printerHDC, ENDDOC, 0, NULL, NULL);

		UE_LOG(JPrinterLog,Log, TEXT("Printing...    wPX=%i hPX=%i"), width, height);

		DeleteDC(printerHDC);
		out = true;
	}

#endif
	
	return out;
}
