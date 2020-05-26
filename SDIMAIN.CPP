//---------------------------------------------------------------------
#include <vcl.h>
#include <direct.h>
#include <stdio.h>
#include <windows.h>
#include <sys/stat.h>
#pragma hdrstop

#include "SDIMain.h"
#include "About.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TSDIAppForm *SDIAppForm;

static HANDLE serialHandle;

AnsiString NRFUTIL = "C:\\Work\\Borland\\FW_File_Converter\\nrfutil.exe";

AnsiString FW_Version;
AnsiString BUILD_Number;

AnsiString path;
AnsiString dat_File_Name;
AnsiString bin_File_Name;

//---------------------------------------------------------------------
__fastcall TSDIAppForm::TSDIAppForm(TComponent *AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------

void __fastcall TSDIAppForm::FileNew1Execute(TObject *Sender)
{
  // Do nothing        
}
//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::FileOpen1Execute(TObject *Sender)
{
  OpenDialog->Execute();        
}
//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::FileSave1Execute(TObject *Sender)
{
  SaveDialog->Execute();        
}
//---------------------------------------------------------------------------


void __fastcall TSDIAppForm::FileExit1Execute(TObject *Sender)
{
  Close();        
}
//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::HelpAbout1Execute(TObject *Sender)
{
  AboutBox->ShowModal();        
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

int ReadFromPipeNoWait( HANDLE hPipe, char* pDest, int nMax )
{
    DWORD nBytesRead= 0;
	DWORD nAvailBytes;
    char cTmp;
	memset( pDest, 0, nMax );
    // -- check for something in the pipe
    PeekNamedPipe( hPipe, &cTmp, 1, NULL, &nAvailBytes, NULL );
    if ( nAvailBytes == 0 ) {
         return( nBytesRead );
    }
    // OK, something there... read it
    ReadFile( hPipe, pDest, nMax-1, &nBytesRead, NULL);
	return( nBytesRead );
}
//---------------------------------------------------------------------------

BOOL ExecAndProcessOutput(wchar_t *szCmd, wchar_t *szParms  )
{
    SECURITY_ATTRIBUTES rSA=    {0};
    rSA.nLength=              sizeof(SECURITY_ATTRIBUTES);
    rSA.bInheritHandle=       TRUE;

    HANDLE hReadPipe, hWritePipe;
    CreatePipe( &hReadPipe, &hWritePipe, &rSA, 25000 );

    PROCESS_INFORMATION rPI= {0};
    STARTUPINFO         rSI= {0};
    rSI.cb=             sizeof(rSI);
	rSI.dwFlags=     STARTF_USESHOWWINDOW |STARTF_USESTDHANDLES;
    rSI.wShowWindow= SW_HIDE;  // or SW_SHOWNORMAL or SW_MINIMIZE
	rSI.hStdOutput=  hWritePipe;
    rSI.hStdError=   hWritePipe;

	//wchar_t *sCmd = L"d:\\Test\\1.bat";
	//wchar_t *sCmd = MainForm->Edit2->Text.c_str();
/*	wchar_t *sCmd2;//[256];
	int len = szCmd.Length();
	szCmd.WideChar(sCmd2,len);
			*/
	BOOL fRet=CreateProcess(NULL,szCmd, NULL, NULL,true,0,0,0, &rSI, &rPI );
    if ( !fRet ) {
		 return( FALSE );
	}
   //------------------------- and process its stdout every 100 ms
   char dest[1000];
   wchar_t *sProgress = L"";
   DWORD dwRetFromWait= WAIT_TIMEOUT;
   while ( dwRetFromWait != WAIT_OBJECT_0 ) {
		dwRetFromWait= WaitForSingleObject( rPI.hProcess, 100 );
		if ( dwRetFromWait == WAIT_ABANDONED ) {  // crash?
            break;
        }
        //--- else (WAIT_OBJECT_0 or WAIT_TIMEOUT) process the pipe data
		while ( ReadFromPipeNoWait( hReadPipe, dest, sizeof(dest) ) > 0 ) {
			SDIAppForm->Memo1->Lines->Add(dest);
        }
    }
	CloseHandle( hReadPipe  );
    CloseHandle( hWritePipe );
    CloseHandle( rPI.hThread);
	CloseHandle( rPI.hProcess);
    // MessageBox("All done!");
    return TRUE;
}
//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::Button1Click(TObject *Sender)
{
	FILE *File_Write;
	FILE *File_dat;
	FILE *File_bin;

	unsigned int counter = 0;
	char buf[1];
	int bytes_read;

	struct stat statbuf;

	AnsiString output_file = "C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\Bootloader_Files\\C3_";
	output_file += FW_Version;
	output_file += '_';
	output_file += BUILD_Number;
	output_file += ".hex";

	File_Write = fopen(output_file.c_str(), "wb");
	File_dat = fopen("C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C_Files\\C3_Version4_B15.dat", "rb");
	File_bin = fopen("C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C_Files\\C3_Version4_B15.bin", "rb");

	fstat(fileno(File_dat), &statbuf);

	unsigned int size = statbuf.st_size;

	if(size > 255)
	{
		while(1){}
	}

	char *write = "M";
	write[0] = 0x09;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x01;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x02;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x07;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x06;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x01;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x01;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x01;
	fwrite(write, 1, 1, File_Write);
	write[0] = (size & 0xFF);//0x8d;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x08;
	fwrite(write, 1, 1, File_Write);

    //Read .dat file and add it to output file
	bool stop = false;

	char const_ch1[1];
	char const_ch2[1];
	const_ch1[0] = 0xC0;
	const_ch2[0] = 0xDB;

	while(stop == false)
	{
		bytes_read = fread(buf, 1, 1, File_dat);		//Read first byte of telegram

		if(bytes_read != 0)
		{
			if(buf[0] == const_ch1[0])
			{
				buf[0] = 0xDB;
				fwrite(buf, 1, 1, File_Write);
				buf[0] = 0xDC;
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}
			else if(buf[0] == const_ch2[0])
			{
				buf[0] = 0xDB;
				fwrite(buf, 1, 1, File_Write);
				buf[0] = 0xDD;
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}
			else
			{
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}

			if(counter >= 0x40)
			{
				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x08;
				fwrite(write, 1, 1, File_Write);
				counter = 0;
			}
		}
		else
			stop = true;
	}

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x03;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x04;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x06;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x02;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x01;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x02;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x10;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x08;
	fwrite(write, 1, 1, File_Write);

    //Read .bin file and add it to output file
	stop = false;
	counter = 0;

	int counter2 = 0;

	fstat(fileno(File_bin), &statbuf);

	size = statbuf.st_size;

	int runs = size / 0x1000;
	unsigned int Last_Size = size % 0x1000;

	int counter3 = 0;

	while(stop == false)
	{
		bytes_read = fread(buf, 1, 1, File_bin);		//Read first byte of telegram

		if(bytes_read != 0)
		{
			if(buf[0] == const_ch1[0])
			{
				buf[0] = 0xDB;
				fwrite(buf, 1, 1, File_Write);
				buf[0] = 0xDC;
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}
			else if(buf[0] == const_ch2[0])
			{
				buf[0] = 0xDB;
				fwrite(buf, 1, 1, File_Write);
				buf[0] = 0xDD;
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}
			else
			{
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}

			if(counter >= 0x40)
			{
				if(counter2 < 0x3F)
				{
					write[0] = 0xC0;
					fwrite(write, 1, 1, File_Write);
					write[0] = 0x08;
					fwrite(write, 1, 1, File_Write);
				}
				counter = 0;
				counter2++;
			}

			if(counter2 == 0x40 && counter3 < (runs-1))		//4096 bytes block
			{
				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x03;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x04;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x01;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x02;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x00;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x10;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x00;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x00;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x08;
				fwrite(write, 1, 1, File_Write);

				counter2 = 0;
				counter3++;
			}
			else if(counter2 == 0x40 && counter3 == (runs-1))		//Last block xxxx bytes block
			{
				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x03;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x04;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x01;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x02;
				fwrite(write, 1, 1, File_Write);
				write[0] = (Last_Size & 0xFF);			//Size
				fwrite(write, 1, 1, File_Write);
				write[0] = ((Last_Size >> 8) & 0xFF);   //Size
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x00;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x00;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x08;
				fwrite(write, 1, 1, File_Write);

				counter2 = 0;
				counter3++;
			}
		}
		else
			stop = true;
	}

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x03;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x04;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);


	fclose(File_dat);
	fclose(File_bin);
	fclose(File_Write);

	Memo1->Lines->Add("Done generating bootloader hex file.");
}
//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::Button2Click(TObject *Sender)
{
//Generates .zip file
	AnsiString cmd = NRFUTIL;
	cmd += " pkg generate --application C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C3_Version4_B15.hex --hw-version 52 --sd-req 0xB6 --application-version 2 --key-file C:\\Work\\Embedded_Code\\DFU_Keys\\private.pem C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C_Files\\C3_App_Package.zip";

	int iSize = cmd.WideCharBufSize();
	wchar_t *sCmd = new wchar_t[iSize];
	cmd.WideChar(sCmd, iSize);

	wchar_t *sParms=NULL;

	BOOL fRet= ExecAndProcessOutput( sCmd, sParms );

	delete[] sCmd;

//Extract .zip file
	cmd = "\"C:\\Program Files\\WinRAR\\winrar.exe\" x -ibck -o+ C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C_Files\\C3_App_Package.zip *.* C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C_Files\\";

	iSize = cmd.WideCharBufSize();
	wchar_t *sCmd2 = new wchar_t[iSize];
	cmd.WideChar(sCmd2, iSize);

	wchar_t *sParms2=NULL;

	fRet= ExecAndProcessOutput( sCmd2, sParms2 );

	delete[] sCmd2;

	Memo1->Lines->Add("Done generating zip file.");

	Button3->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::Button3Click(TObject *Sender)
{
	FILE *File_bin;

	unsigned int counter = 0;
	char buf[10];
	int bytes_read;

	File_bin = fopen("C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C_Files\\C3_Version4_B15.bin", "rb");


	bool stop = false;

	while(stop == false)
	{
		bytes_read = fread(buf, 1, 1, File_bin);		//Read first byte of telegram

		if(buf[0] == 'C')
		{
			bytes_read = fread(buf, 1, 10, File_bin);

			if(buf[0] == '3')
			{
				AnsiString tmp = buf;
				int pos = tmp.Pos("HOLTER");

				if(pos != 0)
				{
					AnsiString Line;
					int index = 0;

					do
					{
						bytes_read = fread(buf, 1, 1, File_bin);

						if(buf[0] == 'B')
						{
							Line = buf[0];
							index = 1;
						}
						else if(index == 1 && buf[0] == '1')
						{
							Line += buf[0];
							index = 2;
						}
						else if(index == 2 && (buf[0] == '8') || (buf[0] == '9'))
						{
							Line += buf[0];
							index = 3;
						}
						else
							index = 0;

					}
					while(index != 3);

					bytes_read = fread(buf, 1, 1, File_bin);

					bytes_read = fread(buf, 1, 5, File_bin);

					FW_Version = buf;
					FW_Version.SetLength(5);

					do
					{
						bytes_read = fread(buf, 1, 1, File_bin);
					}
					while(buf[0] != '3');

					do
					{
						bytes_read = fread(buf, 1, 1, File_bin);
					}
					while(buf[0] == 0x00);

					BUILD_Number = buf[0];

					do
					{
						bytes_read = fread(buf, 1, 1, File_bin);

						if(buf[0] != 0x00)
							BUILD_Number += buf[0];
					}
					while(buf[0] != 0x00);

					stop = true;
					Button1->Enabled = true;
					Button4->Enabled = true;
				}
			}
		}
	}

	fclose(File_bin);

	Memo1->Lines->Add("Done extracting version and build number.");

	Label3->Caption = FW_Version;
	Label5->Caption = BUILD_Number;
}
//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::Button4Click(TObject *Sender)
{
//Generates settings file
	AnsiString cmd = NRFUTIL;
	cmd += " settings generate --application C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C3_Version4_B15.hex --family NRF52840 --application-version 1 --bootloader-version 1 --bl-settings-version 2 C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C_Files\\settings.hex";

	int iSize = cmd.WideCharBufSize();
	wchar_t *sCmd = new wchar_t[iSize];
	cmd.WideChar(sCmd, iSize);

	wchar_t *sParms=NULL;

	BOOL fRet= ExecAndProcessOutput( sCmd, sParms );

	delete[] sCmd;

	Memo1->Lines->Add("Done generating settings file.");

	Button5->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::Button5Click(TObject *Sender)
{
//Merges APP + SETTINGS
	AnsiString cmd = "mergehex -m C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C3_Version4_B15.hex C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C_Files\\settings.hex -o C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C_Files\\merged_app_sett.hex";

	int iSize = cmd.WideCharBufSize();
	wchar_t *sCmd = new wchar_t[iSize];
	cmd.WideChar(sCmd, iSize);

	wchar_t *sParms=NULL;

	BOOL fRet= ExecAndProcessOutput( sCmd, sParms );

	delete[] sCmd;

//Merges (APP + SETTINGS) + (BL + SD)
	cmd = "mergehex -m C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\C_Files\\merged_app_sett.hex C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\merged_BL_SD.hex -o C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\Production_Files\\Production_C3_";
	cmd += FW_Version;
	cmd += '_';
	cmd += BUILD_Number;
	cmd += ".hex";

	iSize = cmd.WideCharBufSize();
	wchar_t *sCmd2 = new wchar_t[iSize];
	cmd.WideChar(sCmd2, iSize);

	wchar_t *sParms2=NULL;

	fRet= ExecAndProcessOutput( sCmd2, sParms2 );

	Memo1->Lines->Add("Done making combined production file.");

	delete[] sCmd2;
}
//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::Button6Click(TObject *Sender)
{
	AnsiString tmp;

	if (OpenDialog->Execute())
	{
		if(OpenDialog->FileName != NULL)
		{
			int pos;

			//dat_File_Name = OpenDialog->FileName;
			tmp = OpenDialog->FileName;

			//Memo1->Lines->Add(dat_File_Name);
			//this->Label7->Caption = dat_File_Name;

			path.SetLength(0);

			do
			{
				pos = tmp.Pos('\\');

				if(pos != 0)
				{
					path += tmp.SubString(1,pos);
					tmp.Delete(1,pos);
				}
			}
			while(pos != 0);

			dat_File_Name = OpenDialog->FileName;
			bin_File_Name = OpenDialog->FileName;

			pos = bin_File_Name.Pos(".dat");
			bin_File_Name.SetLength(pos);
			bin_File_Name += "bin";


			Memo1->Lines->Add(path);
			Memo1->Lines->Add(' ');
			Memo1->Lines->Add(dat_File_Name);
			Memo1->Lines->Add(' ');
			Memo1->Lines->Add(bin_File_Name);
			Memo1->Lines->Add(' ');

            Button7->Enabled = true;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::Button7Click(TObject *Sender)
{
	FILE *File_Write;
	FILE *File_dat;
	FILE *File_bin;

	unsigned int counter = 0;
	char buf[1];
	int bytes_read;

	struct stat statbuf;

	AnsiString output_file = path;//"C:\\Work\\Embedded_Code\\nRF52840_DK\\nRF_SDK_15.3.0_background_dfu_3\\examples\\Cortrium\\C3_Version4_B15\\pca10056\\s140\\ses\\Output\\Debug\\Exe\\Bootloader_Files\\C3_";
	output_file += "Bootloader_out.hex";

	File_Write = fopen(output_file.c_str(), "wb");

	File_dat = fopen(dat_File_Name.c_str(), "rb");
	File_bin = fopen(bin_File_Name.c_str(), "rb");

	fstat(fileno(File_dat), &statbuf);

	unsigned int size = statbuf.st_size;

/*	if(size > 255)
	{
		while(1){}
	}
*/
	char *write = "M";
	write[0] = 0x09;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x01;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x02;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x07;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x06;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x01;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x01;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x01;
	fwrite(write, 1, 1, File_Write);
	write[0] = (size & 0xFF);//0x8d;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x08;
	fwrite(write, 1, 1, File_Write);

    //Read .dat file and add it to output file
	bool stop = false;

	char const_ch1[1];
	char const_ch2[1];
	const_ch1[0] = 0xC0;
	const_ch2[0] = 0xDB;

	while(stop == false)
	{
		bytes_read = fread(buf, 1, 1, File_dat);		//Read first byte of telegram

		if(bytes_read != 0)
		{
			if(buf[0] == const_ch1[0])
			{
				buf[0] = 0xDB;
				fwrite(buf, 1, 1, File_Write);
				buf[0] = 0xDC;
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}
			else if(buf[0] == const_ch2[0])
			{
				buf[0] = 0xDB;
				fwrite(buf, 1, 1, File_Write);
				buf[0] = 0xDD;
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}
			else
			{
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}

			if(counter >= 0x40)
			{
				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x08;
				fwrite(write, 1, 1, File_Write);
				counter = 0;
			}
		}
		else
			stop = true;
	}

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x03;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x04;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x06;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x02;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x01;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x02;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x10;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x00;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x08;
	fwrite(write, 1, 1, File_Write);

    //Read .bin file and add it to output file
	stop = false;
	counter = 0;

	int counter2 = 0;

	fstat(fileno(File_bin), &statbuf);

	size = statbuf.st_size;

	int runs = size / 0x1000;
	unsigned int Last_Size = size % 0x1000;

	int counter3 = 0;

	while(stop == false)
	{
		bytes_read = fread(buf, 1, 1, File_bin);		//Read first byte of telegram

		if(bytes_read != 0)
		{
			if(buf[0] == const_ch1[0])
			{
				buf[0] = 0xDB;
				fwrite(buf, 1, 1, File_Write);
				buf[0] = 0xDC;
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}
			else if(buf[0] == const_ch2[0])
			{
				buf[0] = 0xDB;
				fwrite(buf, 1, 1, File_Write);
				buf[0] = 0xDD;
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}
			else
			{
				fwrite(buf, 1, 1, File_Write);

				counter++;
			}

			if(counter >= 0x40)
			{
				if(counter2 < 0x3F)
				{
					write[0] = 0xC0;
					fwrite(write, 1, 1, File_Write);
					write[0] = 0x08;
					fwrite(write, 1, 1, File_Write);
				}
				counter = 0;
				counter2++;
			}

			if(counter2 == 0x40 && counter3 < (runs-1))		//4096 bytes block
			{
				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x03;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x04;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x01;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x02;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x00;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x10;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x00;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x00;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x08;
				fwrite(write, 1, 1, File_Write);

				counter2 = 0;
				counter3++;
			}
			else if(counter2 == 0x40 && counter3 == (runs-1))		//Last block xxxx bytes block
			{
				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x03;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x04;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x01;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x02;
				fwrite(write, 1, 1, File_Write);
				write[0] = (Last_Size & 0xFF);			//Size
				fwrite(write, 1, 1, File_Write);
				write[0] = ((Last_Size >> 8) & 0xFF);   //Size
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x00;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x00;
				fwrite(write, 1, 1, File_Write);

				write[0] = 0xC0;
				fwrite(write, 1, 1, File_Write);
				write[0] = 0x08;
				fwrite(write, 1, 1, File_Write);

				counter2 = 0;
				counter3++;
			}
		}
		else
			stop = true;
	}

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x03;
	fwrite(write, 1, 1, File_Write);

	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0x04;
	fwrite(write, 1, 1, File_Write);
	write[0] = 0xC0;
	fwrite(write, 1, 1, File_Write);


	fclose(File_dat);
	fclose(File_bin);
	fclose(File_Write);

	Memo1->Lines->Add("Done generating bootloader hex file.");
}
//---------------------------------------------------------------------------

