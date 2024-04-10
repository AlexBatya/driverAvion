#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <stdbool.h>
#include "./include/ini.h"

#define MAX_PORT_NAME_LENGTH 20
#define CONFIG_FILENAME "config.ini"

typedef struct {
	char portName[MAX_PORT_NAME_LENGTH];
	int baudRate;
	int dataBits;
} Config;

bool loadConfig(Config *config) {
  FILE *file = fopen(CONFIG_FILENAME, "r");
  if (file == NULL) {
  	fprintf(stderr, "Ошибка открытия конфигурационного файла\n");
		return false;
  }

  char line[256];
  while (fgets(line, sizeof(line), file) != NULL) {
  	char key[256], value[256];
  	if (sscanf(line, "%[^=]=%[^\n]", key, value) == 2) {
			if (strcmp(key, "portName") == 0) {
				strncpy(config->portName, value, MAX_PORT_NAME_LENGTH);
			} 
			else if (strcmp(key, "baudRate") == 0) {
				config->baudRate = atoi(value);
			} 
			else if (strcmp(key, "dataBits") == 0) {
				config->dataBits = atoi(value);
			}
		}
	}

	fclose(file);
	return true;
}

int main() {
	Config config;
	if (!loadConfig(&config)) {
		return 1;
	}

	// Открытие COM-порта
	HANDLE hSerial = CreateFile(config.portName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hSerial == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Ошибка открытия порта\n");
		return 1;
	}

	// Установка параметров COM-порта
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		fprintf(stderr, "Ошибка получения параметров порта\n");
		CloseHandle(hSerial);
		return 1;
	}

	dcbSerialParams.BaudRate = config.baudRate;
	dcbSerialParams.ByteSize = config.dataBits;
	dcbSerialParams.StopBits = ONESTOPBIT; // 1 стоповый бит
	dcbSerialParams.Parity = NOPARITY; // Без контроля четности

	if (!SetCommState(hSerial, &dcbSerialParams)) {
		fprintf(stderr, "Ошибка установки параметров порта\n");
		CloseHandle(hSerial);
		return 1;
	}

	// Чтение данных с порта и вывод их в консоль
	char buffer[256];
	DWORD bytesRead;
	while (1) {
		if (ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, NULL)) {
			if (bytesRead > 0) {
				printf("Принято %lu байт: ", bytesRead);
				for (DWORD i = 0; i < bytesRead; ++i) {
					printf("%02X ", (unsigned char)buffer[i]);
				}
				printf("\n");
			}
		} 
		else {
			fprintf(stderr, "Ошибка чтения из порта\n");
			CloseHandle(hSerial);
			return 1;
		}
	}

	// Закрытие порта
	CloseHandle(hSerial);
	return 0;
}
