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

//---------------------------------------------------------------------------

void __fastcall TSDIAppForm::Button6Click(TObject *Sender)
{
	AnsiString tmp;

	if (OpenDialog->Execute())
	{
		if(OpenDialog->FileName != NULL)
		{
			int pos;

			tmp = OpenDialog->FileName;

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

