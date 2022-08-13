#include <LiquidCrystal.h>
#include <IRremote.h>
#define tempoSoneca 5
#define pinoMotor 6
#define pinoDetector 2


int tempEscolhida;
bool ligado=false;
bool funcionalidade=true;
bool primeiraVez=true;
bool primeiraVez2=true;
int rotacao;
float tempAmbiente;
int v1,valor;
int seg;

//declarando o receptor
IRrecv receptor(10);
decode_results comando;
//declarando o LCD
//LiquidCrystal lcd(2,3,4,7,8,12,13);
LiquidCrystal lcd(3,4,5,7,8,12,13);
void desligando(){
    ligado=false;

    lcd.clear();
}

void aumentarTemperatura(){
    tempEscolhida++;
    lcd.clear(); 
    lcd.setCursor(6,0);
    lcd.print(tempEscolhida);
}

void diminuirTemperatura(){
    tempEscolhida--;
    lcd.clear(); 
    lcd.setCursor(6,0);
    lcd.print(tempEscolhida);
}

int rpm(){
    float diferencaTemp, RPM;
    diferencaTemp = tempAmbiente - tempEscolhida;

    //funcionar a 10% da capacidade
    if(diferencaTemp == 0){
        RPM = 12.5;
        return RPM;
    }

    //Desligar
    if(diferencaTemp < 0){
        return 0;
    }
    //rpm proporcional a diferença de temperatura
    if(diferencaTemp > 0){
        if(diferencaTemp >= 5){
            return 125;
        }
        if(diferencaTemp < 5){
            RPM =( diferencaTemp / 5.0) * 125;
            return RPM;
        }
    }
  }



void setup(){
  Serial.begin(9600);
    // configurando o receptor I.R
    pinMode(10,INPUT);
    receptor.enableIRIn();
    //saida pwm para o motor
    pinMode(pinoMotor,OUTPUT);
    //detector de movimento
    pinMode(11,INPUT);
    // configurando o LCD
    lcd.begin(16,2);
    //configurando o timer1
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 0x3D09;
    TCCR1B |= (1 << WGM12) | (1 << CS10) | (1 << CS12);
    TIMSK1 |= (1 << OCIE1A);
    attachInterrupt(digitalPinToInterrupt(pinoDetector),troca,RISING);
}

//função responsável por tratar a interrupção
ISR (TIMER1_COMPA_vect){
    if(!funcionalidade && ligado){
        seg++;
       if(seg==tempoSoneca){
            desligando();
            seg=0;
       } 
    }
}

void troca(){
  if(!funcionalidade){
    ligado=true;
    seg=0;
  }
}

void loop(){
	//loop desligado
	while(!ligado){
        //recebendo a temperatura
        valor = analogRead(A0); 
        tempAmbiente = ((165/338.0)*valor) - (8410/169.0); 
      	if(primeiraVez){
      		//arredondando a temperatura
        	tempEscolhida=tempAmbiente;
             primeiraVez=false;
      	}
        //testando o comando
        if(receptor.decode(&comando)){
            switch(comando.value){
                //ligando 
                case(0xFDA05F):
              		lcd.clear();
                    Serial.println(tempAmbiente);
                    ligado = true;
                    lcd.setCursor(6,0);
                  if(primeiraVez2){
                    lcd.print(tempAmbiente);
                    primeiraVez2=false;
                  }else{
                      lcd.print(tempEscolhida); 
                   }
                    break;
                    //botão desliga "soneca" um
                    case(0xFD08F7):{
                        funcionalidade = false;
                        lcd.clear();
                        lcd.print("soneca desligada");
                        break;}
                     
                    //botão liga soneca 0
                     case(0xFD30CF):{
                        funcionalidade = true;
                        lcd.clear();
                        lcd.print("soneca ligada");
                        break;}
              
                  //seta para cima(aumento da temperatura escolhida)
                    case(0xFD50AF):{
                      Serial.print("oi");
                        aumentarTemperatura();
                        break;}
                   //seta para baixo(diminui a temperatura escolhida)
                    case(0xFD10EF):{
                        diminuirTemperatura();
                        break;}
                
            }
         
            receptor.resume();
        }
        analogWrite(pinoMotor,0);
      Serial.print("oi");
     delay(1000);
    }
	//loop ligado
    while(ligado){
			//recebe a temperatura
			valor = analogRead(A0); 
            tempAmbiente = ((165/338.0)*valor) - (8410/169.0);
            if(receptor.decode(&comando)){
            	switch(comando.value){
            		//desligando
                    case(0xFD00FF):{
                    desligando();
                    break;}
                   //seta para cima(aumento da temperatura escolhida)
                    case(0xFD50AF):{
                      Serial.print("oi");
                        aumentarTemperatura();
                      Serial.print("oi")
                ;
                        break;}
                   //seta para baixo(diminui a temperatura escolhida)
                    case(0xFD10EF):{
                        diminuirTemperatura();
                        break;}
                    //botão desliga "soneca" um
                    case(0xFD08F7):{
                        funcionalidade = false;
                        lcd.clear();
                        lcd.print("soneca desligada");
                        break;}
                     
                    //botão liga soneca 0
                     case(0xFD30CF):{
                        funcionalidade = true;
                        lcd.clear();
                        lcd.print("soneca ligada");
                        break;}
                     }
                    
                   receptor.resume();
                   delay(50);
                    }
            rotacao = rpm();
            Serial.print(rotacao);
      
            v1 = rotacao/2.8;
            delay(100);
            analogWrite(pinoMotor,v1);
            
	}
}
