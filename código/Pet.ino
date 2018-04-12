/*Incluindo todas as bibliotecas e coisinhas de um jeito mais bonito*/
#include "all.h"

/*Taxa de decaimento das barras no tempo*/
const int velPerda=1;
const int perdaFome=2;
const int perdaSede=3;
const int perdaHigiene=1;
const int perdaFelicidade=1;

//Modulo RTC DS1307 ligado as portas A4 e A5 do Arduino 
DS1307 rtc(A4, A5);

/*Definindo mais coisas*/
const int buttonPosition[4]={3,4,5,2};
const char saveIntegrity[2]={0,1};
const char save[2]={5,50};
const char timeSaveIntegrity[2]={2,3};
const char timeSave[2]={100,150};
const int daysPerMonth[2][13] = {{-1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                           {-1, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 

#define OLED_RESET 6
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/*Posicoes das barras na tela*/
#define POSXBARRAS 97
#define BARRAVIDA 6
#define BARRAFOME 19
#define BARRASEDE 32
#define BARRAHIGIENE 45
#define BARRAFELICIDADE 58

enum estado 
{
  CLOCK,
  GAME,
  MENU,
  GAMEOVER
};

class Pet
{
  public:
  
    int vida;
    int fome;
    int sede;
    int higiene;
    int felicidade;
    int age;

    int lastIt;

    void calcularParametros(long elapsedSeconds);
    void alimentar();
    void hidratar();
    void limpar();
    void brincar();    
    void createPet();
    
};

class MasterClock
{
    Time lastTime;
    
    int firstIt;
    unsigned long lastIt;
    unsigned long correct;

    void timeStore();
    void timeRestart();
    Time addSecondsToDate(Time indate, long seconds);
    int daysElapsed(Time date0, Time date1);

  public:

    Time returnHours();
    Time initTime;
    MasterClock();
    long secondsElapsed();
    
};

class Animation //idle=0 , sleep=1, eat=2,  drink=3,  bath=4,  none=5,  death=6
{
    int estado;
    int frame;
    unsigned long lastIt;
    
  
  public:

    Animation();
    void sleep ();
    void idle  ();
    void none  ();
    void death ();
    void eat   ();
    void drink ();
    void bath  ();
    void play  ();
    
};

Pet pet; //instancia o objeto pet da classe Pet
MasterClock relogio; //instancia o objeto relogio da classe MasterClock
Animation animation; //instancia o objeto animation da classe Animation
estado GameState=CLOCK; // indica que o estado inicial é o de relógio.

void setup() ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(4,INPUT);
  pinMode(5,INPUT);


  //Aciona o relogio
  rtc.halt(false);
  
  //As linhas abaixo setam a data e hora do modulo
  //e podem ser comentada apos a primeira utilizacao
  /*rtc.setDOW(SATURDAY);      //Define o dia da semana
  rtc.setTime(0, 54, 0);     //Define o horario
  rtc.setDate(25, 11, 2017);   //Define o dia, mes e ano*/
  
  //Definicoes do pino SQW/Out
  rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);

  relogio.initTime=rtc.getTime();
  
  //Inicia a comunicação I2C com o display OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  //Limpa o display
  display.clearDisplay();

  //Mostra as mudanças no display(atualiza a imagem)
  display.display();

  if(digitalRead(5)==0)
  {
    //Verifica a integridade do savo do slot 0
    int slot0=EEPROM.read(saveIntegrity[0]);
    
    //Verifica a integridade do savo do slot 1
    int slot1=EEPROM.read(saveIntegrity[1]);
  
    //se o slot 0 estiver bom, carregar save do slot 0
    if (slot0)
    {
      //carrega o save do slot 0
      readAnything((char*)&pet,sizeof(pet),save[0]);
    }
  
    //se o slot 0 estiver ruim e o 1 bom, carregar save do slot 1
    else if (slot1)
    {
      //carrega o save do slot 1
      readAnything((char*)&pet,sizeof(pet),save[1]);
    }
    //se ambos os slots estiverem corrompidos, criar novo pet 
    else
    {
        //cria um novo pet
        pet.createPet();
    }
  }
  else
  {
    pet.createPet();
  }
  int buttonLastState[3]={0};
  int menuItem=0;
  
  while(1)
  {
      if(digitalRead(buttonPosition[3]))
      {
        GameState=CLOCK;
        pet.createPet();
      }
      int button[3]={0};
      
      for(int i=0 ; i<3 ; i++)
      {
        int buttonRead=digitalRead(buttonPosition[i]);
        if(buttonRead==1 && buttonLastState[i] == 0)
        {
          button[i]=1;
        }
        buttonLastState[i]=buttonRead;
      }
      
      

      long secondsElapsed = relogio.secondsElapsed();
      pet.calcularParametros(secondsElapsed);

      
      if(GameState==CLOCK)
      {
        
        if(pet.vida>0)
        {          
          if(button[1]==1)
          {
            GameState=GAME;
          }
          Time horas = relogio.returnHours();
          mostrarHoras(horas);
          display.clearDisplay();
          animation.sleep();
        }
        else
        {          
          if(button[1]==1)
          {
            GameState=GAMEOVER;
          }
          display.clearDisplay();
          animation.death();
        }
        Time horas = relogio.returnHours();
        mostrarHoras(horas);
      }
      
      else if(GameState==GAME)
      {
        menuItem=0;
        if(pet.vida<=0)
        {
          GameState=GAMEOVER;
        }
        else if(button[1]==1)
        {
          GameState=MENU;
        }
        display.clearDisplay();
        displayStats();
        animation.idle();        
      }
      
      else if(GameState==MENU) // 0 - comida, 1 - água, 2 - banho, 3 - brincar, 4 - sair
      {
        display.clearDisplay();
        opcoesMenu(menuItem);
        if(button[1]==1)
        {
          if(menuItem==0)
          {
            pet.alimentar();
          }
          else if (menuItem==1)
          {
            pet.hidratar();
          }
          else if(menuItem==2)
          {
            pet.limpar();
          }
          else if(menuItem==3)
          {
            pet.brincar();
          }
          GameState=GAME;
        }
        else if(button[2]==1)
        {
          menuItem++;
          if(menuItem>4)
          {
            menuItem=0;
          }
        }
        else if(button[0]==1)
        {
          menuItem--;
          if(menuItem<0)
          {
            menuItem=4;
          }
        }
      }
      
      else if(GameState==GAMEOVER)
      {
        if(button[1]==1)
        {          
          pet.createPet();
          GameState=GAME;
        }
        display.clearDisplay();
        if(button[1]==0)
        {
          displayIdade();
        }
        escrever("GAME OVER", 9, 2, WHITE, 10, 0);
        animation.death();
      }
      
      display.display();
  }
  
}////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{;}

void escrever(char stringue[], short int tamanho ,short int tamanhoFonte, short int cor, short int posX, short int posY)
{
  display.setTextSize(tamanhoFonte);
  display.setTextColor(cor);
  display.setCursor(posX,posY);
  for(int i=0;i<tamanho;i++)
  {
    display.write(stringue[i]);
  }
}

char intToChar (int a) // Função que converte um algarismo int para char
{
  return (a >= 0 && a <= 9)? (a + '0'): '\0';
}

void displayStats()
{
  
  displayBarras();
  displayIdade();
}

void displayIdade()
{
  char idadeDisplay[]="idade:XXXX dias";
  unsigned long idade= pet.age/(86400);
    char s_idade[4]={'\0'};
    for(int i=3;i>=0;i--)
    {
      idadeDisplay[i+6]=intToChar(idade%10);
      idade/=10;
    }
  escrever(idadeDisplay, 15, 1, WHITE, 0, 55);
}
void displayBarras()
{
  display.drawBitmap(POSXBARRAS,0,statBar,16,64,WHITE);
  short int vida=1+pet.vida/500;
  display.fillRect(POSXBARRAS+9, BARRAVIDA, vida, 2, WHITE);
  short int fome=1+pet.fome/500;
  display.fillRect(POSXBARRAS+9, BARRAFOME, fome, 2, WHITE);
  short int sede=1+pet.sede/500;
  display.fillRect(POSXBARRAS+9, BARRASEDE, sede, 2, WHITE);
  short int higiene=1+pet.higiene/500;
  display.fillRect(POSXBARRAS+9, BARRAHIGIENE, higiene, 2, WHITE);
  short int felicidade=1+pet.felicidade/500;
  display.fillRect(POSXBARRAS+9, BARRAFELICIDADE, felicidade, 2, WHITE);
}

void storeAnything(char * thing, int tam, int pos)
{
  for (int i=0;i<tam;i++)
  {
    EEPROM.write(i+pos,0x00);
    EEPROM.write(i+pos,thing[i]);
  }
}

void readAnything(char * thing, int tam, int pos)
{
  for (int i=0;i<tam;i++)
  {
    char * adress = thing + i;
    *adress=EEPROM.read(pos+i);
  }
}

void opcoesMenu(int opcao) // 0 - comida, 1 - água, 2 - banho, 3 - brincar, 4 - sair
{
  if(opcao==0)
  {
    escrever("comida", 6, 2, WHITE, 0, 32);
  }
  if(opcao==1)
  {
    escrever("agua", 4, 2, WHITE, 0, 32);
  }
  if(opcao==2)
  {
    escrever("banho", 5, 2, WHITE, 0, 32);
  }
  if(opcao==3)
  {
    escrever("brincar", 7, 2, WHITE, 0, 32);
  }
  if(opcao==4)
  {
    escrever("sair", 4, 2, WHITE, 0, 32);
  }
}
// Programado por Henrique Finger Zimerman
// Invejosos diro que mentira
// - Eh mentira ~ratatusznei
void mostrarHoras(Time horas)
{
  char hora[]="XX:XX";
  char data[]="YY/YY/YY";

  hora[0]= intToChar(horas.hour/10);
  hora[1]= intToChar(horas.hour%10);

  hora[3]= intToChar(horas.min/10);
  hora[4]= intToChar(horas.min%10);

  data[0]= intToChar(horas.date/10);
  data[1]= intToChar(horas.date%10);

  data[3]= intToChar(horas.mon/10);
  data[4]= intToChar(horas.mon%10);

  data[6]= intToChar((horas.year/10)%10);
  data[7]= intToChar(horas.year%10);
  
  escrever(hora, 5 , 2 , WHITE , 4, 0);
  escrever(data, 8 , 1 , WHITE, 74, 4);
}

void Pet::calcularParametros(long elapsedSeconds)
{
  if(vida>0)
  {
    this->age+=elapsedSeconds;
    this->lastIt+=elapsedSeconds;
    if(this->lastIt>=10)
    {
      for(int i = 0 ; i < this->lastIt/10 ; i++)
      {
          int happyMod;
          if(this->felicidade>=5000)
          {
            happyMod=0;
          }
          else if(this->felicidade>=2500)
          {
            happyMod=1;
          }
          else
          {
            happyMod=2;
          }
          int higiMod;
          if(this->higiene>=5000)
          {
            higiMod=0;
          }
          else if(this->higiene>=2500)
          {
            higiMod=1;
          }
          else
          {
            higiMod=2;
          }
          int fomeMod;
          if(this->fome>=5000)
          {
            fomeMod=0;
          }
          else if(this->fome>=2500)
          {
            fomeMod=1;
          }
          else
          {
            fomeMod=2;
          }
          int sedeMod;
          if(this->sede>=5000)
          {
            sedeMod=0;
          }
          else if(this->sede>=2500)
          {
            sedeMod=1;
          }
          else
          {
            sedeMod=2;
          }
          
          this->fome       -= velPerda*(perdaFome);
          this->sede       -= velPerda*(perdaSede);
          this->higiene    -= velPerda*(perdaHigiene);
          this->felicidade -= velPerda*(perdaFelicidade+higiMod);
          this->vida       -= velPerda*happyMod*(fomeMod+sedeMod);
          this->vida       += velPerda*(2-happyMod)*(2-higiMod)*(4-(fomeMod+sedeMod));
          if(this->vida>10000)
          {
            this->vida=10000;
          }
          if(this->fome<0)
          {
            this->fome=0;
          }
          if(this->sede<0)
          {
            this->sede=0;
          }
          if(this->higiene<0)
          {
            this->higiene=0;
          }
          if(this->felicidade<0)
          {
            this->felicidade=0;
          }
      }
    }
  }
  if(elapsedSeconds>0)
  {
    EEPROM.write(0,0);
    storeAnything((char*)this, sizeof(*this), save[0]);
    EEPROM.write(0,1);
    EEPROM.write(1,0);
    storeAnything((char*)this, sizeof(*this), save[1]);
    EEPROM.write(1,1);
    this->lastIt%=10;
  }
}

void Pet::alimentar()
{
  this->fome = 10000;
  animation.eat();
}

void Pet::hidratar()
{
  this->sede = 10000;
  animation.drink();
}

void Pet::limpar()
{
  this->higiene = 10000;
  animation.bath();
}

void Pet::brincar()
{
  this->felicidade = 10000;
  animation.play();
}

void Pet::createPet()
{
  this->vida       = 10000;
  this->fome       = 10000;
  this->sede       = 10000;
  this->higiene    = 10000;
  this->felicidade = 10000;
  this->age        = 0;
  this->lastIt     = 0;
}

MasterClock::MasterClock()
{


  this->lastIt=millis();
  this->correct=0;
  this->firstIt=1;
  
  int slot0=EEPROM.read(timeSaveIntegrity[0]);
  
  //Verifica a integridade do savo do slot 1
  int slot1=EEPROM.read(timeSaveIntegrity[1]);

  Time lastTime;
  
  if (slot0)
  { 
    readAnything((char*)&lastTime,sizeof(lastTime),timeSave[0]);
  }

  else if (slot1)
  {
    readAnything((char*)&lastTime,sizeof(lastTime),timeSave[1]);
  }
  this->lastTime=lastTime;
}

Time MasterClock::returnHours()
{
  return this->lastTime;
}

Time MasterClock::addSecondsToDate(Time indate, long seconds)
{

  

  seconds=indate.sec+seconds;

  if(seconds>=60)
  {
    indate.min+=seconds/60;    
  }
  indate.sec=seconds%60;

  if(indate.min>=60)
  {

    indate.hour+=indate.min/60;
    indate.min%=60;
  }
  if(indate.hour>=24)
  {

    indate.date+=indate.hour/24;
    indate.hour%=24;
  }
  int BiYear=0;
  if(indate.year%4==0)
  {
    BiYear=1;
  }
  while(indate.date>daysPerMonth[BiYear][indate.mon])
  {
    indate.date-=daysPerMonth[BiYear][indate.mon];
    indate.mon+=1;
    if(indate.mon>=13)
    {
      indate.mon=1;
      indate.year+=1;
    }
    if(indate.year%4==0)
    {
      BiYear=1;
    }
    else
    {
      BiYear=0;
    }
  }
  return indate;
}

int MasterClock::daysElapsed(Time date0, Time date1)
{
  int BiYear=0;
  if(date0.year%4==0)
  {
    BiYear=1;
  }
  int dayCount=0;
  while(date0.date != date1.date || date0.mon != date1.mon || date0.year != date1.year)
  {
    date0.date++;
    dayCount++;
    if(date0.date>daysPerMonth[BiYear][date0.mon])
    {
      date0.date=1;
      date0.mon++;
    }
    if(date0.mon>=13)
    {
      date0.mon=1;
      date0.year+=1;
    }
    if(date0.year%4==0)
    {
      BiYear=1;
    }
    else
    {
      BiYear=0;
    }
    if(dayCount>1000)
      break;
  }
  return dayCount;
}

long MasterClock::secondsElapsed()
{
  long elapsedSeconds=0;
  
  Time lastTime = this->lastTime;

  if(this-> firstIt==1)
  {
    Time initTime = this->initTime;
    int daysElapsed = this->daysElapsed(lastTime,initTime);
    elapsedSeconds = daysElapsed * 86400;

    int daySecondsInit, daySecondsLast;
    daySecondsInit = 3600 * initTime.hour + 60 * initTime.min + initTime.sec;
    daySecondsLast = 3600 * lastTime.hour + 60 * lastTime.min + lastTime.sec;
    elapsedSeconds = elapsedSeconds + (daySecondsInit - daySecondsLast);
    
  }
  
  if(millis()<this->lastIt)
  {
    this->lastIt=4294967295-this->lastIt;
  }
  if (millis()+this->correct-this->lastIt>=1000)
  {
    elapsedSeconds+=(millis()+this->correct-this->lastIt)/1000;
    this->correct=(millis()+this->correct-this->lastIt)%1000;
    lastIt=millis();
  }  
  
  
  if(elapsedSeconds>0 && firstIt==1)
  {
    this->lastTime=initTime;
    this->timeStore();
  }
  if(elapsedSeconds>0 && firstIt==0)
  {
    this->lastTime=addSecondsToDate(this->lastTime,elapsedSeconds);
    this->timeStore();
  }
  this->firstIt = 0;
  
  
  return elapsedSeconds;
}

void MasterClock::timeStore()
{
  Time lastTime=this->lastTime;
  EEPROM.write(timeSaveIntegrity[0],0);
  storeAnything((char*)&lastTime, sizeof(lastTime), timeSave[0]);
  EEPROM.write(timeSaveIntegrity[0],1);
  EEPROM.write(timeSaveIntegrity[1],0);
  storeAnything((char*)&lastTime, sizeof(lastTime), timeSave[1]);
  EEPROM.write(timeSaveIntegrity[1],1);

}

Animation::Animation()
{
  this->estado=8;
  this->lastIt=0;
  this->frame=0;
}

void Animation::sleep () //idle=0 , sleep=1, eat=2,  drink=3,  bath=4,  none=5,  death=6
{
  if(this->estado==1)
  {
    if(millis()-this->lastIt >= 500)
    {
      lastIt=millis();
      if(this->frame==1)
      {
        this->frame=2;
      }
      else if(this->frame==2)
      {
        this->frame=1;
      }
    }
    if(this->frame==1)
    {
      display.drawBitmap(36,30,dormindo1,56,32,WHITE);
    }
    else if(this->frame==2)
    {
      display.drawBitmap(36,30,dormindo2,56,32,WHITE);
    }
  }
  else
  {
    this->estado=1;
    this->lastIt=millis();
    this->frame=1;
    display.drawBitmap(36,30,dormindo1,56,32,WHITE);
  } 
}
void Animation::idle  ()
{
  if(this->estado==0)
  {
    if(millis()-this->lastIt >= 500)
    {
      lastIt=millis();
      if(this->frame==1)
      {
        this->frame=2;
      }
      else if(this->frame==2)
      {
        this->frame=3;
      }
      else if(this->frame==3)
      {
        this->frame=4;
      }
      else if(this->frame==4)
      {
        this->frame=1;
      }
    }
    if(this->frame==1)
    {
      display.drawBitmap(20,0,idle1,64,64,WHITE);
    }
    else if(this->frame==2)
    {
      display.drawBitmap(20,0,idle2,64,64,WHITE);
    }
    else if(this->frame==3)
    {
      display.drawBitmap(20,0,idle3,64,64,WHITE);
    }
    else if(this->frame==4)
    {
      display.drawBitmap(20,0,idle2,64,64,WHITE);
    }
  }
  else
  {
    this->estado=0;
    this->lastIt=millis();
    this->frame=1;
    display.drawBitmap(20,0,idle1,64,64,WHITE);
  } 
}
void Animation::none  ()
{
   this->estado=5;
}
void Animation::death (){;}

void Animation::eat   ()
{
  display.clearDisplay();
  display.drawBitmap(32 , 8 , alimento1 , 64 , 48 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();  
  display.drawBitmap(32 , 8 , alimento2 , 64 , 48 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(32 , 8 , alimento3 , 64 , 48 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(32 , 8 , alimento4 , 64 , 48 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();    
}
void Animation::drink ()
{
  display.clearDisplay();
  display.drawBitmap(32 , 8 , bebida1 , 64 , 48 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(32 , 8 , bebida2 , 64 , 48 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(32 , 8 , bebida2 , 64 , 48 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(32 , 8 , bebida2 , 64 , 48 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
}

void Animation::play ()
{
  display.clearDisplay();
  display.drawBitmap(32 , 8 , brincando1 , 64 , 64 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(32 , 8 , brincando2 , 64 , 64 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(32 , 8 , brincando3 , 64 , 64 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(32 , 8 , brincando4 , 64 , 64 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
}

void Animation::bath  ()
{
  display.clearDisplay();
  display.drawBitmap(36 , 0 , banho1 , 56 , 64 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(36 , 0 , banho2 , 56 , 64 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(36 , 0 , banho1 , 56 , 64 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
  display.drawBitmap(36 , 0 , banho2 , 56 , 64 , WHITE);
  display.display();
  delay(500);
  display.clearDisplay();
}

