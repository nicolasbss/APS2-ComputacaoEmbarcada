/*
 * tipos.h
 *
 * Created: 06/05/2019 17:22:59
 *  Author: Nicolas Stegmann
 */ 


#ifndef TIPOS_H_
#define TIPOS_H_


 typedef struct {
	 const uint8_t *data;
	 uint16_t width;
	 uint16_t height;
	 uint8_t dataSize;
 } tImage;

 typedef struct {
	 uint32_t x;
	 uint32_t y;
	 uint32_t size_x;
	 uint32_t size_y;
	 tImage *image;
	 void (*p_handler)(void);
 } botao;

 typedef struct{
	 uint16_t x;
	 uint16_t y;
	 tImage *image;
 }imagem;
 
 typedef struct {
	 long int code;
	 const tImage *image;
 } tChar;
 
 typedef struct {
	 int length;
	 const tChar *chars;
	 char start_char;
	 char end_char;
 } tFont;


#endif /* TIPOS_H_ */