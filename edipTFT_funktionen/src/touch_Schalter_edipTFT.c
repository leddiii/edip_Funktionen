

/*****************************************************************************************************************************************************************************
 *****************************************************************************************************************************************************************************
|*
|*Autor: p. Schwarz
|*
|*Function: edipTFT_touch_schalter
|*
|*Description:
|*
|*Zeichnet einen Touch-Schalter im Bereich xx1,yy1,xx2,yy2. Der downcode und der upcode wird in den Sendepuffer übertragen,wenn der Schalter betätigt wird.
|*Der Text wird in "" als string übergeben. Über die variabe "rahmen", kann ein gewünschter Rahmen gezeichnet werden. Die möglichkeiten für einen der
|*18 Rahmen, finden Sie in der Bedienungsanleitung für das edip240 auf der Seite 23.
|*
|*Übergabeparameter:
|*
|*dev_adr: 	Adressierungsbyte des Slaves.
|*x1:		ertser Punkt auf der X-Achse des Displays für den Button					(0-479)
|*y1:		erster Punkt auf der Y-Achse des Displays für den Schalter					(0-271)
|*x2:		zweiter Punkt auf der X-Achse des Displays für den Button					(0-479)
|*y2:		zweiter Punkt auf der Y-Achse des Displays für den Schalter					(0-271)
|*downcode:	Übergabewert in den Sendepuffer, wenn der Schalter nicht betätigt ist 		(1-255)
|*upcode:	Übergabewert in den Sendepuffer, wenn der Schalter betätigt ist 			(1-255)
|*Text:		Beschriftungstext !!!!!Muss in "Anführungsstrichen" stehen!!!!!				(max.46 Zeichen)
|*rahmen:	Art des gewünschten Rahmens siehe Bedienungsanleitung Seite 23				(1-20)
|*
|*Beispiel des aufrufes:
|*edipTFT_touch_schalter(0xDE,100,50,140,68,171,205,"Ein",3); Zeichnet einen Touchschalter fast mittig mit den Returnwerten 171(Schalter gefallen)
|*und 205(Schalter betätigt) mit der Bezeichnung "EIN"
*****************************************************************************************************************************************************************************
 *****************************************************************************************************************************************************************************
 */


uint8_t edipTFT_touch_schalter(uint8_t dev_adr,uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t downcode, uint8_t upcode, char *Text, uint8_t rahmen)
{
    uint8_t Sendbuf[64];
    uint8_t ack[2];
    uint8_t sum = 0;
    uint8_t ret;
    int len = 16;
    int str_len = 0;
    int arr_len = (int)strlen(Text) + len; // die Plus vier für die Rahmen einstellung
    Sendbuf[0] = 0x11;
    Sendbuf[2] = 0x1B;
    Sendbuf[3] = 0x41;
    Sendbuf[4] = 0x4B;
    Sendbuf[5] = (uint8_t)x1;
    Sendbuf[6] = x1 >> 8;
    Sendbuf[7] = (uint8_t)y1;
    Sendbuf[8] = y1 >> 8;
    Sendbuf[9] = (uint8_t)x2;
    Sendbuf[10] = x2 >> 8;
    Sendbuf[11] = (uint8_t)y2;
    Sendbuf[12] = y2 >> 8;
    Sendbuf[13] = downcode;
    Sendbuf[14] = upcode;
    Sendbuf[15] = 0x43; // C für text begin
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
|*Autor: P. Schwarz
|*
|*Function: edipTFT_SchalterStellung
|*
|*Description:
|*
|*Fragt den Sendepuffer des Gerätes ab und gibt den down-und upcode des Schalter zurück. Da der Sendepuffer am besten über ein Interrupt abgefragt wird,
|*empfiehlt es sich, die Funktion im Interrupt aufzurufen.
|*
|*Übergabeparameter:
|*
|*dev_adr: 	Adressierungsbyte des Slaves.
|*
|*Beispiel des aufrufes:
|*
|*uint8_t Schalter1			// variable für Schalter.
|*
|*Schalter1=edipTFT_SchalterStellung(0xDE) Diese funktion sollte so im Interrupt aufgerufen werden. Da die funktion einen Wert zurück gibt,
|*bekommt die Variable "Schalter1" den Wert des down- oder upcodes zugewiesen.
*****************************************************************************************************************************************************************************
 *****************************************************************************************************************************************************************************
*/

uint8_t edipTFT_SchalterStellung (uint8_t dev_adr)
{
	uint8_t bcc=0;
	uint8_t ret=0;
	uint8_t ack[1];
	uint8_t Sendeanfrage[4];
	uint8_t Rueckgabewert[8];
	uint8_t Pruefsumme=0;

	Sendeanfrage[0]=0x12;
	Sendeanfrage[1]=0x01;
	Sendeanfrage[2]=0x53;

	for (int i=0;i<=2;i++)
	{
		bcc += Sendeanfrage[i];
	}

	Sendeanfrage[3]=bcc;

	ret=HAL_I2C_IsDeviceReady(&hi2c1, dev_adr, 3, 10);
	printf("ret=: 3%i",ret);
	if(ret==0)
	{
		HAL_I2C_Master_Transmit(&hi2c1, dev_adr, Sendeanfrage, 4, 0xFF);
		HAL_I2C_Master_Receive(&hi2c1, dev_adr, ack, 1, 10);
		HAL_I2C_Master_Receive(&hi2c1,dev_adr,Rueckgabewert,8,10);

	}

	for(int p=1;p<=6;p++)
	{
		Pruefsumme += Rueckgabewert[p];
	}

	printf("Pruefsumme= %2x \n",Pruefsumme);

	if (Pruefsumme == Rueckgabewert[7] && Rueckgabewert[4] == 0x41)
	{
		return Rueckgabewert[6];
	}

	HAL_Delay(50);
}
