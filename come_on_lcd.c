#include "come_on.h"

#define ST7586_SPI_INSTANCE	0

#define ST_COMMAND			0
#define ST_DATA				1

#define RATIO_SPI0_LCD_SCK	4
#define RATIO_SPI0_LCD_A0		28
#define RATIO_SPI0_LCD_MOSI	29
#define RATIO_SPI0_LCD_BSTB	30
#define RATIO_SPI0_LCD_CS		31

static const nrf_drv_spi_t st7586_spi = NRF_DRV_SPI_INSTANCE(ST7586_SPI_INSTANCE);
static volatile bool st7586_spi_xfer_done = false;
static unsigned char rx_data;
static int i, j;

static void spi_event_handler(nrf_drv_spi_evt_t const *p_event, void *p_context) {
	st7586_spi_xfer_done = true;
}

static void st7586_write(const uint8_t category, const uint8_t data) {
	int err_code;
	nrf_gpio_pin_write(RATIO_SPI0_LCD_A0, category);

	st7586_spi_xfer_done = false;
	err_code = nrf_drv_spi_transfer(&st7586_spi, &data, 1, &rx_data, 0);
	APP_ERROR_CHECK(err_code);
	while (!st7586_spi_xfer_done) {
		__WFE();
	}
	nrf_delay_us(10);
}

void lcd_init() {
	// spi setup
	int err_code;
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.ss_pin   = RATIO_SPI0_LCD_CS;
	spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED;
	spi_config.mosi_pin = RATIO_SPI0_LCD_MOSI;
	spi_config.sck_pin  = RATIO_SPI0_LCD_SCK;
	spi_config.frequency = NRF_SPI_FREQ_1M;
	spi_config.mode = NRF_DRV_SPI_MODE_3;

	err_code = nrf_drv_spi_init(&st7586_spi, &spi_config, spi_event_handler, NULL);
	APP_ERROR_CHECK(err_code);

	nrf_gpio_pin_set(RATIO_SPI0_LCD_A0);
	nrf_gpio_cfg_output(RATIO_SPI0_LCD_A0);

	nrf_gpio_pin_clear(RATIO_SPI0_LCD_BSTB);
	nrf_gpio_cfg_output(RATIO_SPI0_LCD_BSTB);

	// lcd 리셋
	nrf_gpio_pin_write(RATIO_SPI0_LCD_BSTB, 0);
	nrf_delay_ms(50);
	nrf_gpio_pin_write(RATIO_SPI0_LCD_BSTB, 1);

	nrf_delay_ms(10);
	nrf_delay_ms(120);
	st7586_write(ST_COMMAND, 0xD7);		// Disable Auto Read
	st7586_write(ST_DATA, 0x9F);
	st7586_write(ST_COMMAND, 0xE0);		// Enable OTP Read
	st7586_write(ST_DATA, 0x00);
	nrf_delay_ms(10);
	st7586_write(ST_COMMAND, 0xE3);		// OTP Up-Load
	nrf_delay_ms(20);
	st7586_write(ST_COMMAND, 0xE1);		// OTP Control Out
	st7586_write(ST_COMMAND, 0x11);		// Sleep Out
	st7586_write(ST_COMMAND, 0x28);		// Display OFF
	nrf_delay_ms(50);

	// lms에서 부팅 수정한 부분(전압 관련)
	st7586_write(ST_COMMAND,  0xC0);
	st7586_write(ST_DATA, 0x53);
	st7586_write(ST_DATA, 0x01);
	st7586_write(ST_COMMAND,  0xC3);
	st7586_write(ST_DATA, 0x02);
	st7586_write(ST_COMMAND,  0xC4);
	st7586_write(ST_DATA, 0x06);

	st7586_write(ST_COMMAND, 0xD0);		// Enable Analog Circuit
	st7586_write(ST_DATA, 0x1D);
	st7586_write(ST_COMMAND, 0xB5);		// N-Line = 0
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_COMMAND, 0x39);		// Monochrome Mode
	st7586_write(ST_COMMAND, 0x3A);		// Enable DDRAM Interface
	st7586_write(ST_DATA, 0x02);
	st7586_write(ST_COMMAND, 0x36);		// Scan Direction Setting
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_COMMAND, 0xB0);		// Duty Setting
	st7586_write(ST_DATA, 0x9F);
	st7586_write(ST_COMMAND, 0xB4);		// Partial Display
	st7586_write(ST_DATA, 0xA0);
	st7586_write(ST_COMMAND, 0x30);		// Partial Display Area = COM0 ~ COM119
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x77);
	st7586_write(ST_COMMAND, 0x20);		// Display Inversion OFF

	st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x2A);		// 0~42(128px / 3)
	st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 0x9F);		// 0~159(160px)

	// Clear whole DDRAM by "0"
	st7586_write(ST_COMMAND, 0x2C);
	for(i = 0; i < 160; i++){
		for(j = 0; j < 43; j++){
			st7586_write(ST_DATA, 0x00);
		}
	}

	st7586_write(ST_COMMAND, 0x29);		// display on
}

void draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
	//그릴 범위 지정
	st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, x1);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, x2);
	st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, y1);
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, y2);

	//직접 그리기
	//행 160, 열 128
	
	st7586_write(ST_COMMAND, 0x2C);
	for(i = 0; i < 160; i++){
		for(j = 0; j < 128; j++){
			st7586_write(ST_DATA, 0xFF);
		}
	}
}

//플레이어 1: (0,6)~(47,13), 플레이어 2: (80, 6)~(127, 13)
void draw_hp(uint8_t pl, uint8_t hp)
{
	int i1, j1;
	if (pl == 1) {
		//체력바 범위 설정
		st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 0);
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 15);
		st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 6);
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 13);

		st7586_write(ST_COMMAND, 0x2C);
		for (i1 = 6; i1 < 14; i1++)	//행 색칠
		{
			//흰색 색칠
			for (j1 = 0; j1 < 15 - hp / 3; j1++)
			{
				st7586_write(ST_DATA, 0x00);
			}
			//j==15-hp/3
			if (hp % 3 == 1) {
				st7586_write(ST_DATA, 0x03);
				j1++;
			}
			else if (hp % 3 == 2) {
				st7586_write(ST_DATA, 0x1F);
				j1++;
			}
			//검은색 색칠
			for (; j1 < 16; j1++)
			{
				st7586_write(ST_DATA, 0xFF);
			}

		}
		
	}
	else if (pl == 2) {
		//체력바 범위 설정
		st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
		st7586_write(ST_DATA, 0x00);	
		st7586_write(ST_DATA, 26);			//26
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 42);			//42
		st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 6);
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 13);

		st7586_write(ST_COMMAND, 0x2C);
		for (i1 = 6; i1 < 14; i1++)	//행 색칠
		{
			if (hp > 0)
			{
				st7586_write(ST_DATA, 0x03);
				//hp--;
				//검은색 색칠
				for (j1 = 1; j1 < hp / 3+1 ; j1++)
				{
					st7586_write(ST_DATA, 0xFF);
				}
				//j==hp/3 16
				/*if (hp % 3 == 0) {
					st7586_write(ST_DATA, 0xFF);
					j1++;
				}*/
				if (hp % 3 == 2) {
					st7586_write(ST_DATA, 0xE0);
					j1++;
				}
				else if (hp % 3 == 1) {
					st7586_write(ST_DATA, 0x00);
					j1++;
				}
				//흰색 색칠
				for (; j1 < 17; j1++)
				{
					st7586_write(ST_DATA, 0x00);
				}
			}
			else
			{
				for (j1 = 0; j1 < 127 / 3; j1++)
				{
					st7586_write(ST_DATA, 0x00);
				}
			}
		}
		
	}
}

void draw_circle(uint8_t x1, uint8_t wl)
{
	static const uint8_t empty_circle[] = {
		0x00, 0x00, 0x00,
		0x00, 0x1F, 0x00,
		0x03, 0xE0, 0xFC,
		0x03, 0x00, 0x1C,
		0x1C, 0x00, 0x03,
		0x1C, 0x00, 0x03,
		0x03, 0x00, 0x1C,
		0x03, 0xE0, 0xFC,
		0x00, 0x1F, 0x00,
		0x00, 0x00, 0x00
	};
	static const uint8_t filled_circle[] = {
		0x00, 0x00, 0x00,
		0x00, 0x1F, 0x00,
		0x03, 0xFF, 0xFC,
		0x03, 0xFF, 0xFC,
		0x1F, 0xFF, 0xFF,
		0x1F, 0xFF, 0xFF,
		0x03, 0xFF, 0xFC,
		0x03, 0xFF, 0xFC,
		0x00, 0x1F, 0x00,
		0x00, 0x00, 0x00
	};
	if (x1 == 0)
	{
		st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 0);			//여기서
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 2);			//여기까지 (둘 다 포함)
		st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 20);			//여기서
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 29);			//여기까지 (둘 다 포함)
	}
	else if (x1 == 1)
	{
		st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 3);			//여기서
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 5);			//여기까지 (둘 다 포함)
		st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 20);			//여기서
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 29);			//여기까지 (둘 다 포함)
	}
	else if (x1 == 2)
	{
		st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 36);			//여기서
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 38);			//여기까지 (둘 다 포함)
		st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 20);			//여기서
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 29);			//여기까지 (둘 다 포함)
	}
	else if (x1 == 3)
	{
		st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 39);			//여기서
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 41);			//여기까지 (둘 다 포함)
		st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 20);			//여기서
		st7586_write(ST_DATA, 0x00);
		st7586_write(ST_DATA, 29);			//여기까지 (둘 다 포함)
	}
	if (wl == 0) {
		st7586_write(ST_COMMAND, 0x2C);
		for (i = 0; i < 10 * 3; i++) {
			st7586_write(ST_DATA, empty_circle[i]);
		}
	}
	else if (wl == 1) {
		st7586_write(ST_COMMAND, 0x2C);
		for (i = 0; i < 10 * 3; i++) {
			st7586_write(ST_DATA, filled_circle[i]);
		}
	}
}

void draw_ready()
{
	static const uint8_t draw_READY[] = {
		0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xff, 0xff, 0xff, 0xfc, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x03, 0xe0, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0x03, 0xe0, 0x00, 0x03, 0xe0, 0x03, 0xe0, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x03, 0xe0, 0x03, 0xe0, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x03, 0xe0, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x1f, 0x00, 0x00,
		0xfc, 0x00, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x03, 0xe0, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x1f, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00,
		0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00,
		0xfc, 0x00, 0xfc, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00,
		0xfc, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00,
		0xfc, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00,
		0xfc, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00,
		0xfc, 0x00, 0x03, 0xe0, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00,
		0xfc, 0x00, 0x03, 0xe0, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00,
		0xfc, 0x00, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00,
		0xfc, 0x00, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0xfc, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00
	};

	st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 9);			//여기서
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 33);			//여기까지 (둘 다 포함)
	st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 40);			//여기서
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 59);			//여기까지 (둘 다 포함)

	st7586_write(ST_COMMAND, 0x2C);
	for (i = 0; i < 20 * 25; i++) {
		st7586_write(ST_DATA, draw_READY[i]);
	}
}

void draw_go()
{
	static const uint8_t draw_GO[]={
		0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00,
		0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00,
		0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0xff, 0xff, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0xff, 0xff, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xfc, 0x00, 0x00, 0x1f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0x00,
		0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00,
		0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00
	};

	st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 16);			//여기서
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 26);			//여기까지 (둘 다 포함)
	st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 40);			//여기서
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 59);			//여기까지 (둘 다 포함)

	st7586_write(ST_COMMAND, 0x2C);
	for (i = 0; i < 20 * 11; i++) {
		st7586_write(ST_DATA, draw_GO[i]);
	}
}
void exer1()
{
	int i1, j1;
	st7586_write(ST_COMMAND, 0x2A);		// Column Address Setting
	st7586_write(ST_DATA, 0x00);		
	st7586_write(ST_DATA, 26);			//여기서
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 42);			//여기까지 (둘 다 포함)
	st7586_write(ST_COMMAND, 0x2B);		// Row Address Setting
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 6);			//여기서
	st7586_write(ST_DATA, 0x00);
	st7586_write(ST_DATA, 13);			//여기까지 (둘 다 포함)

	st7586_write(ST_COMMAND, 0x2C);
	for (i1 = 6; i1 < 10; i1++) {
		st7586_write(ST_DATA, 0x03);
		for (j1 = 1; j1 < 16; j1++) {
			st7586_write(ST_DATA, 0xFF);
			//st7586_write(ST_DATA, 0xFF);
		}
		st7586_write(ST_DATA, 0x00);
		
	}
}