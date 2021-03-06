/**
 * \file
 *
 * \brief Example of usage of the maXTouch component with USART
 *
 * This example shows how to receive touch data from a maXTouch device
 * using the maXTouch component, and display them in a terminal window by using
 * the USART driver.
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage
 *
 * \section intro Introduction
 * This simple example reads data from the maXTouch device and sends it over
 * USART as ASCII formatted text.
 *
 * \section files Main files:
 * - example_usart.c: maXTouch component USART example file
 * - conf_mxt.h: configuration of the maXTouch component
 * - conf_board.h: configuration of board
 * - conf_clock.h: configuration of system clock
 * - conf_example.h: configuration of example
 * - conf_sleepmgr.h: configuration of sleep manager
 * - conf_twim.h: configuration of TWI driver
 * - conf_usart_serial.h: configuration of USART driver
 *
 * \section apiinfo maXTouch low level component API
 * The maXTouch component API can be found \ref mxt_group "here".
 *
 * \section deviceinfo Device Info
 * All UC3 and Xmega devices with a TWI module can be used with this component
 *
 * \section exampledescription Description of the example
 * This example will read data from the connected maXTouch explained board
 * over TWI. This data is then processed and sent over a USART data line
 * to the board controller. The board controller will create a USB CDC class
 * object on the host computer and repeat the incoming USART data from the
 * main controller to the host. On the host this object should appear as a
 * serial port object (COMx on windows, /dev/ttyxxx on your chosen Linux flavour).
 *
 * Connect a terminal application to the serial port object with the settings
 * Baud: 57600
 * Data bits: 8-bit
 * Stop bits: 1 bit
 * Parity: None
 *
 * \section compinfo Compilation Info
 * This software was written for the GNU GCC and IAR for AVR.
 * Other compilers may or may not work.
 *
 * \section contactinfo Contact Information
 * For further information, visit
 * <A href="http://www.atmel.com/">Atmel</A>.\n
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "maquina1.h"

#define MAX_ENTRIES        3
#define STRING_LENGTH     70

#define USART_TX_MAX_LENGTH     0xff
#define COLOR_BLUE           (0x0000FFu)
#define COLOR_GREEN          (0x00FF00u)
#define COLOR_RED            (0xFF0000u)
#define COLOR_NAVY           (0x000080u)
#define COLOR_DARKCYAN       (0x008B8Bu)
#define COLOR_TURQUOISE      (0x40E0D0u)

struct ili9488_opt_t g_ili9488_display_opt;
  
#include "conf_board.h"
#include "conf_example.h"
#include "conf_uart_serial.h"
#include "calibri_36.h"
#include "arial_72.h"
#include "icones/centri.h"
#include "icones/heavy.h"
#include "icones/icon_backward.h"
#include "icones/icon_forward.h"
#include "icones/next.h"
#include "icones/play.h"
#include "icones/previous.h"
#include "icones/wash.h"
#include "icones/water.h"
#include "icones/lock.h"
#include "icones/unlocked.h"
#include "icones/locked.h"
#include "coordenadas.h"


#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  11
#define BUT_IDX_MASK (1 << BUT_IDX)

botao numero_exagues;
botao numero_centri;
botao bubbles;
botao heavy;
botao but_back;
botao but_play;
botao but_next;
botao but_lock;

volatile bool lock_flag = true;
volatile t_ciclo *ciclo_atual;
volatile int numero_de_botoes = 8;

volatile bool flag_porta_aberta = false;
volatile int f_rtt_alarme = 0;
volatile int f_but_back = 0;
volatile int f_but_next = 0;
volatile int f_but_play = 0;
volatile int f_but_nexagues = 0;
volatile int f_but_ncentri = 0;
volatile int f_but_bubbles = 0;
volatile int f_but_heavy = 0;

void draw_cicle(void);

void numero_exagues_callback(void) {
	f_but_nexagues = 1;
}

void numero_centri_callback(void) {
	f_but_ncentri = 1;
}

void bubbles_callback(void) {
	f_but_bubbles = 1;
}

void heavy_callback(void) {
	f_but_heavy = 1;
}

void but_play_callback(void) {
	f_but_play = 1;
}

void but_back_callback(void) {
	f_but_back = 1;
}

void but_next_callback(void) {
	f_but_next = 1;
}

void but_lock_callback(void) {
	if(lock_flag){
	but_lock.image = &unlocked;
	lock_flag = false;
	numero_de_botoes = 8;
	}
	else{
		but_lock.image = &locked;
		lock_flag = true;
		numero_de_botoes = 1;
	}
	
	ili9488_draw_pixmap(but_lock.x,
	but_lock.y,
	but_lock.image->width,
	but_lock.image->height,
	but_lock.image->data);
	
}

void but_callback(void){
	uint32_t color;
	if (flag_porta_aberta)
	{
		color = COLOR_GREEN;
		} else{
		color = COLOR_RED;
	}
	flag_porta_aberta = !flag_porta_aberta;
	ili9488_set_foreground_color(COLOR_CONVERT(color));
	ili9488_draw_filled_circle(445, 30, 20);
}


void io_init(void)
{
	// Inicializa clock do perif�rico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);

	// Configura PIO para lidar com o pino do bot�o como entrada
	// com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);

	// Configura interrup��o no pino referente ao botao e associa
	// fun��o de callback caso uma interrup��o for gerada
	// a fun��o de callback � a: but_callback()
	pio_handler_set(BUT_PIO,
	BUT_PIO_ID,
	BUT_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but_callback);

	// Ativa interrup��o
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais pr�ximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
}

void font_draw_text(tFont *font, const char *text, int x, int y, int spacing) {
	char *p = text;
	while(*p != NULL) {
		char letter = *p;
		int letter_offset = letter - font->start_char;
		if(letter <= font->end_char) {
			tChar *current_char = font->chars + letter_offset;
			ili9488_draw_pixmap(x, y, current_char->image->width, current_char->image->height, current_char->image->data);
			x += current_char->image->width + spacing;
		}
		p++;
	}
}
	
static void configure_lcd(void){
	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);
}

int processa_touch(botao *b, botao *rtn, int N ,uint32_t x, uint32_t y ){
	for (int i=0; i<N; i++){
		if (((x >= b->x) && (x <= b->x + b->size_x)) && ((y >= b->y) && (y <= b->y + b->size_y))){
			*rtn = *b;
			return 1;
		}
		b++;
	}
	return 0;
}

t_ciclo *initMenuOrder(){
	c_rapido.previous = &c_enxague;
	c_rapido.next = &c_diario;

	c_diario.previous = &c_rapido;
	c_diario.next = &c_pesado;

	c_pesado.previous = &c_diario;
	c_pesado.next = &c_enxague;

	c_enxague.previous = &c_pesado;
	c_enxague.next = &c_centrifuga;

	c_centrifuga.previous = &c_enxague;
	c_centrifuga.next = &c_rapido;

	return(&c_diario);
}

void draw_info() {
	
	char Q[512];
	char C[512];
	char B[512];
	char H[512];
	
	sprintf(Q, "x%d",  ciclo_atual->enxagueQnt);
	sprintf(C, "x%d", ciclo_atual->centrifugacaoTempo);
	sprintf(B, "%d", ciclo_atual->bubblesOn);
	sprintf(H, "%d", ciclo_atual->heavy);
	
	
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
		ili9488_draw_filled_rectangle(80, 40, 150, 80);
		ili9488_draw_filled_rectangle(80, 120, 150, 160);
		ili9488_draw_filled_rectangle(80, 200, 150, 240);
		ili9488_draw_filled_rectangle(80, 280, 150, 320);
		
		ili9488_set_foreground_color(COLOR_BLACK);		
		ili9488_draw_string(Q_X,Q_Y, Q);
		ili9488_draw_string(C_X, C_Y, C);
		ili9488_draw_string(B_X, B_Y, B);
		ili9488_draw_string(H_X, H_Y, H);
	
}

void draw_timer(int x, int y, int tempo) {
	char A[512];
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(x, y, x+270, y+40);
	
	sprintf(A, "Tempo de lavagem: %d min", tempo);
	
	ili9488_set_foreground_color(COLOR_BLACK);
	ili9488_draw_string(x, y, A);
}

void draw_background(void) {
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
}

void draw_botao(botao *but) {
	ili9488_draw_pixmap(but->x, but->y, but->image->width, but->image->height, but->image->data);
}

void draw_screen(void) {
	draw_botao(&numero_exagues);
	draw_botao(&numero_centri);
	draw_botao(&bubbles);
	draw_botao(&heavy);
	draw_botao(&but_back);
	draw_botao(&but_next);
	draw_botao(&but_play);
	draw_botao(&but_lock);
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_RED));
	ili9488_draw_filled_circle(445, 30, 20);
}

void draw_screen1(void) {
	ili9488_draw_pixmap(numero_exagues.x,
	numero_exagues.y,
	numero_exagues.image->width,
	numero_exagues.image->height,
	numero_exagues.image->data);
	
	ili9488_draw_pixmap(numero_centri.x,
	numero_centri.y,
	numero_centri.image->width,
	numero_centri.image->height,
	numero_centri.image->data);
	
	ili9488_draw_pixmap(bubbles.x,
	bubbles.y,
	bubbles.image->width,
	bubbles.image->height,
	bubbles.image->data);
	
	ili9488_draw_pixmap(heavy.x,
	heavy.y,
	heavy.image->width,
	heavy.image->height,
	heavy.image->data);
	
	ili9488_draw_pixmap(but_back.x,
	but_back.y,
	but_back.image->width,
	but_back.image->height,
	but_back.image->data);
	
	ili9488_draw_pixmap(but_next.x,
	but_next.y,
	but_next.image->width,
	but_next.image->height,
	but_next.image->data);
	
	ili9488_draw_pixmap(but_play.x,
	but_play.y,
	but_play.image->width,
	but_play.image->height,
	but_play.image->data);
	
	ili9488_draw_pixmap(but_lock.x,
	but_lock.y,
	but_lock.image->width,
	but_lock.image->height,
	but_lock.image->data);
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_RED));
	ili9488_draw_filled_circle(445, 30, 20);

}

void draw_cicle(void) {
	
	char F[512];
	sprintf(F, "Ciclo atual: %s", ciclo_atual->nome);
	
	
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(160, 130, 420, 170);

	
	ili9488_set_foreground_color(COLOR_BLACK);
	ili9488_draw_string(160, 130, F);	
	draw_timer(160, 170, ciclo_atual->centrifugacaoTempo+ciclo_atual->enxagueTempo);
	
	
}

uint32_t convert_axis_system_x(uint32_t touch_y) {
	// entrada: 4096 - 0 (sistema de coordenadas atual)
	// saida: 0 - 320
	return ILI9488_LCD_HEIGHT - ILI9488_LCD_HEIGHT*touch_y/4096;
}

uint32_t convert_axis_system_y(uint32_t touch_x) {
	// entrada: 0 - 4096 (sistema de coordenadas atual)
	// saida: 0 - 320
	return ILI9488_LCD_WIDTH - ILI9488_LCD_WIDTH*touch_x/4096;
}

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));
	
	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 0);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN);
}

static void mxt_init(struct mxt_device *device)
{
	enum status_code status;

	/* T8 configuration object data */
	uint8_t t8_object[] = {
		0x0d, 0x00, 0x05, 0x0a, 0x4b, 0x00, 0x00,
		0x00, 0x32, 0x19
	};

	/* T9 configuration object data */
	uint8_t t9_object[] = {
		0x8B, 0x00, 0x00, 0x0E, 0x08, 0x00, 0x80,
		0x32, 0x05, 0x02, 0x0A, 0x03, 0x03, 0x20,
		0x02, 0x0F, 0x0F, 0x0A, 0x00, 0x00, 0x00,
		0x00, 0x18, 0x18, 0x20, 0x20, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x02,
		0x02
	};

	/* T46 configuration object data */
	uint8_t t46_object[] = {
		0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x03,
		0x00, 0x00
	};
	
	/* T56 configuration object data */
	uint8_t t56_object[] = {
		0x02, 0x00, 0x01, 0x18, 0x1E, 0x1E, 0x1E,
		0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E,
		0x1E, 0x1E, 0x1E, 0x1E, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00
	};

	/* TWI configuration */
	twihs_master_options_t twi_opt = {
		.speed = MXT_TWI_SPEED,
		.chip  = MAXTOUCH_TWI_ADDRESS,
	};

	status = (enum status_code)twihs_master_setup(MAXTOUCH_TWI_INTERFACE, &twi_opt);
	Assert(status == STATUS_OK);

	/* Initialize the maXTouch device */
	status = mxt_init_device(device, MAXTOUCH_TWI_INTERFACE,
			MAXTOUCH_TWI_ADDRESS, MAXTOUCH_XPRO_CHG_PIO);
	Assert(status == STATUS_OK);

	/* Issue soft reset of maXTouch device by writing a non-zero value to
	 * the reset register */
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_COMMANDPROCESSOR_T6, 0)
			+ MXT_GEN_COMMANDPROCESSOR_RESET, 0x01);

	/* Wait for the reset of the device to complete */
	delay_ms(MXT_RESET_TIME);

	/* Write data to configuration registers in T7 configuration object */
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 0, 0x20);
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 1, 0x10);
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 2, 0x4b);
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 3, 0x84);

	/* Write predefined configuration data to configuration objects */
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_GEN_ACQUISITIONCONFIG_T8, 0), &t8_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_TOUCH_MULTITOUCHSCREEN_T9, 0), &t9_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_SPT_CTE_CONFIGURATION_T46, 0), &t46_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_PROCI_SHIELDLESS_T56, 0), &t56_object);

	/* Issue recalibration command to maXTouch device by writing a non-zero
	 * value to the calibrate register */
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_COMMANDPROCESSOR_T6, 0)
			+ MXT_GEN_COMMANDPROCESSOR_CALIBRATE, 0x01);
}

void RTT_Handler(void)
{
	uint32_t ul_status;

	/* Get RTT status */
	ul_status = rtt_get_status(RTT);
	


	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		f_rtt_alarme = 1;
	}
}

void mxt_handler(struct mxt_device *device, botao *botoes, int Nbotoes)
{
	int last_status;
	/* USART tx buffer initialized to 0 */
	char tx_buf[STRING_LENGTH * MAX_ENTRIES] = {0};
	uint8_t i = 0; /* Iterator */

	/* Temporary touch event data struct */
	struct mxt_touch_event touch_event;

	/* Collect touch events and put the data in a string,
	 * maximum 2 events at the time */
	do {
		/* Temporary buffer for each new touch event line */
		char buf[STRING_LENGTH];
	
		/* Read next next touch event in the queue, discard if read fails */
		if (mxt_read_touch_event(device, &touch_event) != STATUS_OK) {
			continue;
		}
		
		 // eixos trocados (quando na vertical LCD)
		uint32_t conv_y = convert_axis_system_x(touch_event.y);
		uint32_t conv_x = convert_axis_system_y(touch_event.x);
		
		/* Format a new entry in the data string that will be sent over USART */
		sprintf(buf, "Nr: %1d, X:%4d, Y:%4d, Status:0x%2x conv X:%3d Y:%3d\n\r",
				touch_event.id, touch_event.x, touch_event.y,
				touch_event.status, conv_x, conv_y);
	
		
		touch_event.id, touch_event.x, touch_event.y,
		touch_event.status, conv_x, conv_y;
		
		last_status = touch_event.status;
		
		if (last_status < 60) {
			botao but_atual;
			if (processa_touch(botoes, &but_atual, Nbotoes, conv_x, conv_y)){
				but_atual.p_handler();
			}
		}
		
		/* Add the new string to the string buffer */
		strcat(tx_buf, buf);
		i++;
		break;
		/* Check if there is still messages in the queue and
		 * if we have reached the maximum numbers of events */
	} while ((mxt_is_message_pending(device)) & (i < MAX_ENTRIES));

	/* If there is any entries in the buffer, send them over USART */
	if (i > 0) {
		usart_serial_write_packet(USART_SERIAL_EXAMPLE, (uint8_t *)tx_buf, strlen(tx_buf));
	}
}

void config_buttons(){
	numero_exagues.x = 10;
	numero_exagues.y = 10;
	numero_exagues.size_x = 60;
	numero_exagues.size_y = 60;
	numero_exagues.image = &water;
	numero_exagues.p_handler = numero_exagues_callback;
	
	numero_centri.x = 10;
	numero_centri.y = 90;
	numero_centri.size_x = 60;
	numero_centri.size_y = 60;
	numero_centri.image = &recyclewater;
	numero_centri.p_handler = numero_centri_callback;
	
	bubbles.x = 10;
	bubbles.y = 170;
	bubbles.size_x = 60;
	bubbles.size_y = 60;
	bubbles.image = &wash;
	bubbles.p_handler = bubbles_callback;
	
	heavy.x = 10;
	heavy.y = 250;
	heavy.size_x = 60;
	heavy.size_y = 60;
	heavy.image = &tumbledry;
	heavy.p_handler = heavy_callback;
	
	but_play.x = 250;
	but_play.y = 230;
	but_play.size_x = 100;
	but_play.size_y = 80;
	but_play.image = &forwardbuttonformultimedia;
	but_play.p_handler = but_play_callback;
	
	but_back.x = 130;
	but_back.y = 230;
	but_back.size_x = 100;
	but_back.size_y = 80;
	but_back.image = &icon_backward;
	but_back.p_handler = but_back_callback;
	
	but_next.x = 370;
	but_next.y = 230;
	but_next.size_x = 100;
	but_next.size_y = 80;
	but_next.image = &icon_forward;
	but_next.p_handler = but_next_callback;
	
	but_lock.x = 360;
	but_lock.y = 10;
	but_lock.size_x = 60;
	but_lock.size_y = 60;
	but_lock.image = &unlocked;
	but_lock.p_handler = but_lock_callback;
	
}

int main(void)
{
	struct mxt_device device; /* Device data container */

	/* Initialize the USART configuration struct */
	const usart_serial_options_t usart_serial_options = {
		.baudrate     = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength   = USART_SERIAL_CHAR_LENGTH,
		.paritytype   = USART_SERIAL_PARITY,
		.stopbits     = USART_SERIAL_STOP_BIT
	};
	
	int timer = 100000;

	sysclk_init(); /* Initialize system clocks */
	board_init();  /* Initialize board */
	io_init();
	configure_lcd();
	draw_background();
	config_buttons();
	/* Initialize the mXT touch device */
	mxt_init(&device);
	draw_screen();
	ciclo_atual = initMenuOrder();
	ciclo_atual = ciclo_atual->next;
	draw_info();
	
	/* Initialize stdio on USART */
	stdio_serial_init(USART_SERIAL_EXAMPLE, &usart_serial_options);

	printf("\n\rmaXTouch data USART transmitter\n\r");
		
	const botao botoes[8] = {but_lock, but_play, numero_centri, numero_exagues, bubbles, heavy, but_next, but_back};
	
	
	uint16_t pllPreScale = (int) (((float) 32768) / 1.0);
	uint32_t irqRTTvalue  = 60; // 1 minuto
	
	draw_cicle();
	
	while (true) {
		/* Check for any pending messages and run message handler if any
		 * message is found in the queue */
		if (!flag_porta_aberta)
		{
		if (mxt_is_message_pending(&device)) {
			mxt_handler(&device, botoes, numero_de_botoes);
		}
		}
		
		if (f_but_back) {
			ciclo_atual=ciclo_atual->previous;
			draw_cicle();
			draw_info();
			f_but_back = 0;
		}
		
		if (f_but_next) {
			ciclo_atual=ciclo_atual->previous;
			draw_cicle();
			draw_info();
			f_but_next = 0;
		}
		
		if (f_but_play) {
			RTT_init(pllPreScale, 1);
			timer = ciclo_atual->enxagueTempo + ciclo_atual->centrifugacaoTempo;
			
			ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
			ili9488_draw_filled_rectangle(160, 170, 450, 200);
			
			f_but_play = 0;
		}
		
		if (f_rtt_alarme){
					
			// reinicia RTT para gerar um novo IRQ
			RTT_init(pllPreScale, irqRTTvalue);
			
			timer -= 1;
			draw_timer(150, 130, timer);
			/*
			* CLEAR FLAG
			*/
			f_rtt_alarme = 0;
		}
		
		if (f_but_nexagues) {
			
			if (ciclo_atual->enxagueQnt <=6) {
				ciclo_atual->enxagueQnt += 1;
			} else {
				ciclo_atual->enxagueQnt = 0;
			}
			
			draw_info();
			f_but_nexagues = 0;
		}
		
		if (f_but_ncentri) {
			
			if (ciclo_atual->centrifugacaoTempo <= 12) {
				ciclo_atual->centrifugacaoTempo += 1;
				} else {
				ciclo_atual->centrifugacaoTempo = 0;
			}
			
			draw_info();
			f_but_ncentri = 0;
		}
		
		if (f_but_bubbles) {
			
			if (ciclo_atual->bubblesOn) {
				ciclo_atual->bubblesOn = 0;
				} else {
				ciclo_atual->bubblesOn = 1;
			}
			
			draw_info();
			f_but_bubbles = 0;
		}
		
		if (f_but_heavy) {
			
			if (ciclo_atual->heavy) {
				ciclo_atual->heavy = 0;
				} else {
				ciclo_atual->heavy = 1;
			}
			
			draw_info(0);
			f_but_heavy = 0;
		}
		
		
	}

	return 0;
}
