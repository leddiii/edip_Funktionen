

//includes:
#include "touch_Schalter.h"
#include "string.h"
#include "stdio.h"


/*****************************************************************************************************************************************************************************
 *****************************************************************************************************************************************************************************
|*
|*Autor: N. Lethaus
|*
|*Function: edip240_touch_schalter
|*
|*Description:
|*
|*Zeichnet einen Touch-Schalter im Bereich x1,y1,x2,y2. Der downcode und der upcode wird in den Sendepuffer �bertragen,wenn der Schalter bet�tigt wird.
|*Der Text wird in "" als string �bergeben. �ber die variabe "rahmen", kann ein gew�nschter Rahmen gezeichnet werden. Die m�glichkeiten f�r einen der
|*18 Rahmen, finden Sie in der Bedienungsanleitung f�r das edip240 auf der Seite 10.
|*
|*�bergabeparameter:
|*
|*dev_adr: 	Adressierungsbyte des Slaves.
|*x1:		ertser Punkt auf der X-Achse des Displays f�r den Button					(0-239)
|*y1:		erster Punkt auf der Y-Achse des Displays f�r den Schalter					(0-127)
|*x2:		zweiter Punkt auf der X-Achse des Displays f�r den Button					(0-239)
|*y2:		zweiter Punkt auf der Y-Achse des Displays f�r den Schalter					(0-127)
|*downcode:	�bergabewert in den Sendepuffer, wenn der Schalter nicht bet�tigt ist 		(1-255)
|*upcode:	�bergabewert in den Sendepuffer, wenn der Schalter bet�tigt ist 			(1-255)
|*Text:		Beschriftungstext !!!!!Muss in "Anf�hrungsstrichen" stehen!!!!!				(max.46 Zeichen)
|*rahmen:	Art des gew�nschten Rahmens siehe Bedienungsanleitung Seite 10				(1-18)
|*
|*Beispiel des aufrufes:
|*edip240_touch_schalter(0xDE,100,50,140,68,171,205,"Ein",3); Zeichnet einen Touchschalter fast mittig mit den Returnwerten 171(Schalter gefallen)
|*und 205(Schalter bet�tigt) mit der Bezeichnung "EIN"
*****************************************************************************************************************************************************************************
 *****************************************************************************************************************************************************************************
 */
uint8_t edip240_touch_schalter(uint8_t dev_adr, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t downcode, uint8_t upcode, char *Text, uint8_t rahmen)
{
    uint8_t Sendbuf[64];
    uint8_t ack[2];
    uint8_t ret;
    uint8_t sum = 0;
    uint8_t len = 12;
    int str_len = 0;
    uint8_t arr_len = (uint8_t)strlen(Text) + len;
    Sendbuf[0] = 0x11;
    Sendbuf[2] = 0x1B;
    Sendbuf[3] = 0x41;
    Sendbuf[4] = 0x4B;
    Sendbuf[5] = x1;
    Sendbuf[6] = y1;
    Sendbuf[7] = x2;
    Sendbuf[8] = y2;
    Sendbuf[9] = downcode;
    Sendbuf[10] = upcode;
    Sendbuf[11] = 0x43; // C f�r text begin
    for (int a = len; a < arr_len + 1; a++)
    {
    	Sendbuf[a] = (uint8_t)Text[str_len];
        if (str_len != strlen(Text) - 1)
        {
            str_len++;
        }
        else
        {
        	Sendbuf[a + 1] = 0x00;
        	Sendbuf[a + 2] = 0x1B;
        	Sendbuf[a + 3] = 0x41;
        	Sendbuf[a + 4] = 0x45;
        	Sendbuf[a + 5] = rahmen;
            arr_len = arr_len + 5;
            break;
        }
    }
    Sendbuf[1] = arr_len - 2;
    for (int b = 0; b < arr_len; b++)
    {
        sum = sum + Sendbuf[b];
    }
    Sendbuf[arr_len] = sum;

    ret=HAL_I2C_IsDeviceReady(&hi2c1, dev_adr, 3, 10);
    if(ret==0)
    {
    	HAL_I2C_Master_Transmit(&hi2c1, dev_adr, Sendbuf, 64, 0xFF);
    	HAL_I2C_Master_Receive(&hi2c1, dev_adr, ack, 1, 0xFF);
    	HAL_Delay(50);
    }

    return ret;
}


/*****************************************************************************************************************************************************************************
 *****************************************************************************************************************************************************************************
|*
|*Autor: N. Lethaus
|*
|*Function: edip240_SchalterStellung
|*
|*Description:
|*
|*Fragt den Sendepuffer des Ger�tes ab und gibt den down-und upcode des Schalter zur�ck. Da der Sendepuffer am besten �ber ein Interrupt abgefragt wird,
|*empfiehlt es sich, die Funktion im Interrupt aufzurufen.
|*
|*�bergabeparameter:
|*
|*dev_adr: 	Adressierungsbyte des Slaves.
|*
|*Beispiel des aufrufes:
|*
|*uint8_t Schalter1			// variable f�r Schalter.
|*
|*Schalter1=edip240_SchalterStellung(0xDE) Diese funktion sollte so im Interrupt aufgerufen werden. Da die funktion einen Wert zur�ck gibt,
|*bekommt die Variable "Schalter1" den Wert des down- oder upcodes zugewiesen.
*****************************************************************************************************************************************************************************
 *****************************************************************************************************************************************************************************
*/

uint8_t edip240_SchalterStellung (uint8_t dev_adr)
{
	uint8_t bcc=0;
	uint8_t ack[1];
	uint8_t Sendeanfrage[4];
	uint8_t Rueckgabewert[7];
	uint8_t Pruefsumme=0;

	Sendeanfrage[0]=0x12;
	Sendeanfrage[1]=0x01;
	Sendeanfrage[2]=0x53;
	for (int i=0;i<=2;i++)
	{
		bcc += Sendeanfrage[i];
	}

	Sendeanfrage[3]=bcc;

	HAL_I2C_Master_Transmit(&hi2c1, dev_adr, Sendeanfrage, 4, 0xFF);
	HAL_I2C_Master_Receive(&hi2c1, dev_adr, ack, 1, 0xFF);
	HAL_I2C_Master_Receive(&hi2c1,dev_adr,Rueckgabewert,7,0xFF);



	for(int p=0;p<=5;p++)
	{
		Pruefsumme += Rueckgabewert[p];
	}

	if (Pruefsumme == Rueckgabewert[6] && Rueckgabewert[3] == 0x41)
	{
		return Rueckgabewert[5];
	}
	else
	{
	}
}
