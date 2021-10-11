
#include <RF24.h> 

uint8_t juniorByte(uint16_t count);
uint8_t olderByte(uint16_t count);
uint16_t collectByte(uint8_t junior,uint8_t older);

enum mode{ Stop, Start};

class Translation{
	public:
		Translation();
		
		void MODE_TX(uint8_t MODE, RF24 * radio);// Старт/Стоп
		uint8_t MODE_CHECK(RF24 * radio);
		void PackageNum(uint16_t num, RF24 * radio);
		void Transaction(uint16_t num, RF24 * radio, uint8_t * array);
		uint16_t GetPackageNum(RF24 * radio);
		void GetTransaction(RF24 * radio, uint8_t * array, uint16_t len);
		void CRC_SEND(RF24 * radio, uint8_t * array, int len);
		bool CRC_CHECK(RF24 * radio, uint8_t * array, uint8_t len);
		
};

Translation::Translation(){
	
}

void Translation::MODE_TX(uint8_t MODE, RF24 * radio){
	uint8_t Key[1] = {0};
	switch(MODE){
		case Stop: 
				Key[0] = 99; 
				break;
		case Start: 
				Key[0] = 15;
				break;
	}
	radio->write(&Key,1);
  
}

uint8_t Translation::MODE_CHECK(RF24 * radio){
	uint8_t key[1];
	radio->read(&key, 1);
	//Start
	if(key[0] == 15){	
			return 1;
		}
	//Stop
	else if(key[0] == 99){
			return 2;
		}
	//else
	else{
		return 0;
		
	}
}
//Отправка количества пакетов
void Translation::PackageNum(uint16_t num, RF24 * radio){
	//Определение и отправка размера данных
	uint8_t Num[2];
	Num[0] = juniorByte(num);
	Num[1] = olderByte(num);
	radio->write(&Num,2);
}
//Принятие количества пакетов
uint16_t Translation::GetPackageNum(RF24 * radio){
	uint8_t Num[2];
	radio->read(&Num, 2);
	uint16_t Collect = collectByte(Num[0],Num[1]);
	return Collect;
}

//Отправка данных
void Translation::Transaction(uint16_t num, RF24 * radio, uint8_t * array){
	int Remainder = num % 32;//Остаток от деления
    int Division = num / 32;
    uint8_t  Select = 0;
	//Определение количества пакетов отправки
	if(!Remainder){
      Select = Division;
    }
    else if(Remainder){
      Select = Division + 1;
    }
	//Serial.println(Select);
	//Формирование отправного буфера
    uint8_t Buffer[Select][32];
	//Упаковываем наш буфер
	int count = 0;
	for(int i = 0; i < Select;i++){
		for(int j = 0; j < 32;j++){
			if(count > num - 1){
				break;
			}
			Buffer[i][j] = array[count];
			count++;
		}
		radio->write(&Buffer[i],32);
	}
	
}

//Принятие данных
void Translation::GetTransaction(RF24 * radio, uint8_t * array, uint16_t len){
	
	int Remainder = len % 32;//Остаток от деления
    int Division = len / 32;
    uint8_t Select = 0;
	if(!Remainder){
      Select = Division;
    }
    else if(Remainder){
      Select = Division + 1;
    }
	uint8_t Buffer[Select][32];
	int count = 0;
	for(int i = 0; i < Select;i++){	
		radio->read(&Buffer[i],32);

		for(int j = 0; j < 32;j++){
			if(count > len - 1){
				break;
			}
			array[count] = Buffer[i][j]; 
			count++;
		}
	}
	
	//Чтение
	

}

void Translation::CRC_SEND(RF24 * radio, uint8_t * array, int len){
	uint16_t CRC = 0;
	uint8_t CRC16[2];
	uint8_t ASK[1] = {0};
	for(int i = 0;i < len;i++){
		CRC += array[i];
	}
	CRC16[0] = juniorByte(CRC);
	CRC16[1] = olderByte(CRC);
	
	//Serial.println(collectByte(CRC16[0],CRC16[1]));
	radio->write(&CRC16, 2);
	//Ждём ответ
	int count = 0;
	
}

bool Translation::CRC_CHECK(RF24 * radio, uint8_t * array, uint8_t len){
	uint8_t Num[2];
	radio->read(&Num, 2);
	uint16_t Collect = collectByte(Num[0],Num[1]);
	uint16_t CRC = 0;
	
	uint8_t answer[2] = {55, 22};
	
	for(int i = 0;i < 32;i++){
		CRC += array[i];
	}
	radio->stopListening();
	
	if(CRC > 0){
		if(CRC == Collect){
		radio->write(answer[0], 1);
		return true;
		}
		else{
			radio->write(answer[0], 1);
			return false;
		}
		
	}
	else{
		return false;
	}
	radio->startListening();
}

/***********************************************************************/
uint8_t juniorByte(uint16_t count){
  uint8_t take = 0;
  for(uint8_t i = 0; i < 7; i++){
     if(count & 1)take |= (1<<i);
     else take &= ~(1<<i);
     count >>= 1;
  }
  return take;
}

uint8_t olderByte(uint16_t count){
  count >>= 7;
  uint8_t take = 0;
  for(uint8_t i = 0; i < 7; i++){
     if(count & 1)take |= (1<<i);
     else take &= ~(1<<i);
     count >>= 1;
  }
  return take;
}

uint16_t collectByte(uint8_t junior,uint8_t older){
  uint16_t take;
  for(uint8_t i = 0; i < 7; i++){
    if(junior & 1)take |= (1<<i);
    else take &= ~(1<<i);
    junior >>= 1;
  }
  for(uint8_t i = 7; i < 15; i++){
    if(older & 1)take |= (1<<i);
    else take &= ~(1<<i);
    older >>= 1;
  }
  return take;
}

#endif
