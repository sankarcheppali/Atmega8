#ifndef ANALOGPI_H
#define ANALOGPI_H
void digitalResponse(uint8_t response);
void analogResponse(uint16_t analog);
void sendResponse(uint8_t *data);
void errorResponse();
#endif
