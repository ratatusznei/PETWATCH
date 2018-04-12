#ifndef __ALL_H
#define __ALL_H

#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS1307.h>
#include "sprites.h"

/*Funcoes do projeto*/
/*Obrigado, Finger*/
void escrever(char stringue[], short int tamanho ,short int tamanhoFonte, short int cor, short int posX, short int posY);
char intToChar(int a);
void displayStats();
void displayBarras();
void displayIdade();
void storeAnything(char * thing, int tam, int pos);
void readAnything(char * thing, int tam, int pos);
void opcoesMenu(int opcao);
void mostrarHoras();

#endif /*__ALL_H*/
