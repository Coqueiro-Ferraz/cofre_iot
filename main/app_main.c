//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                       _              //
//               _    _       _      _        _     _   _   _    _   _   _        _   _  _   _          //
//           |  | |  |_| |\| |_| |\ |_|   |\ |_|   |_| |_| | |  |   |_| |_| |\/| |_| |  |_| | |   /|    //    
//         |_|  |_|  |\  | | | | |/ | |   |/ | |   |   |\  |_|  |_| |\  | | |  | | | |_ | | |_|   _|_   //
//                                                                                       /              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
*   Programa básico para controle da placa durante a Jornada da Programação 1
*   Permite o controle das entradas e saídas digitais, entradas analógicas, display LCD e teclado. 
*   Cada biblioteca pode ser consultada na respectiva pasta em componentes
*   Existem algumas imagens e outros documentos na pasta Recursos
*   O código principal pode ser escrito a partir da linha 86
*/

// Área de inclusão das bibliotecas
//-----------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ioplaca.h"   // Controles das Entradas e Saídas digitais e do teclado
#include "lcdvia595.h" // Controles do Display LCD
#include "hcf_adc.h"   // Controles do ADC
#include "MP_hcf.h"   // Controles do ADC

// Área das macros
//-----------------------------------------------------------------------------------------------------------------------
# define PWD 1234


// Área de declaração de variáveis e protótipos de funções
//-----------------------------------------------------------------------------------------------------------------------

static const char *TAG = "Placa";
static uint8_t entradas, saidas = 0; //variáveis de controle de entradas e saídas
int controle = 0;
int senha = 0;
int tempo = 50;
int coluna = 0;
uint32_t adcvalor = 0;
char exibir [40];
// Funções e ramos auxiliares
//-----------------------------------------------------------------------------------------------------------------------
void abrir()
{
    int tentativas = 10;
    while(adcvalor <= 170 && tentativas > 0)
    {
        rotacionar_DRV(1,adcvalor<70?100:10,saidas);

        hcf_adc_ler(&adcvalor);                
        adcvalor = adcvalor*360/4095;
        sprintf(exibir,"%"PRIu32"  ", adcvalor);
        lcd595_write(1,0,exibir);
        entradas = io_le_escreve (saidas);
        tentativas--;
    }
}

void fechar()
{
    int tentativas = 10;
    while(adcvalor >= 40 && tentativas > 0)
    {
        rotacionar_DRV(0,adcvalor>140?100:10,saidas);
        hcf_adc_ler(&adcvalor);
        adcvalor = adcvalor*360/4095;
        sprintf(exibir,"%"PRIu32"  ", adcvalor);
        lcd595_write(1,0,exibir);
        entradas = io_le_escreve (saidas);
        tentativas--;
    }
}
void erro ()
{
    lcd595_clear();
    lcd595_write(1,0,"Nao autorizado");
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
}

void restaura()
{
    senha = 0;
    lcd595_clear();
    lcd595_write(2,0, "[ ] [ ] [ ] [ ]");
    controle = 0;
    coluna = 0;
}
// Programa Principal
//-----------------------------------------------------------------------------------------------------------------------

void app_main(void)
{
    /////////////////////////////////////////////////////////////////////////////////////   Programa principal


    // a seguir, apenas informações de console, aquelas notas verdes no início da execução
    ESP_LOGI(TAG, "Iniciando...");
    ESP_LOGI(TAG, "Versão do IDF: %s", esp_get_idf_version());

    /////////////////////////////////////////////////////////////////////////////////////   Inicializações de periféricos (manter assim)
    
    // inicializar os IOs e teclado da placa
    ioinit();      
    entradas = io_le_escreve(saidas); // Limpa as saídas e lê o estado das entradas

    // inicializar o display LCD 
    lcd595_init();
    lcd595_write(1,1,"   Jornada 1   ");
    lcd595_write(2,1,"   COFRE HCF   ");
    
    // Inicializar o componente de leitura de entrada analógica
    esp_err_t init_result = hcf_adc_iniciar();
    if (init_result != ESP_OK) {
        ESP_LOGE("MAIN", "Erro ao inicializar o componente ADC personalizado");
    }

    //delay inicial
    hcf_adc_ler(&adcvalor);                
    adcvalor = adcvalor*360/4095;
    if(adcvalor>50)fechar();
    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    lcd595_clear();
    
    DRV_init(6,7);
    /////////////////////////////////////////////////////////////////////////////////////   Periféricos inicializados


    lcd595_write(2,0, "[ ] [ ] [ ] [ ]");

    /////////////////////////////////////////////////////////////////////////////////////   Início do ramo principal                    
    while (1)                                                                                                                         
    {                                                                                                                                 
        //_______________________________________________________________________________________________________________________________________________________ //
        //-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - -  -  -  -  -  -  -  -  -  -  Escreva seu código aqui!!! //
        //int valor = adc1_get_raw(ADC_CHANNEL_0)*360/4095;
        
        char tecla = le_teclado();
        hcf_adc_ler(&adcvalor);                
        adcvalor = adcvalor*360/4095;

        if (tecla >= '0' && tecla <= '9')
        {
            senha = senha * 10 + tecla - '0';
            controle++;
            switch (controle)
            {
            case 1: lcd595_write(2,0, "[*] [ ] [ ] [ ]");
                break;
            case 2: lcd595_write(2,0, "[*] [*] [ ] [ ]");
                break;
            case 3: lcd595_write(2,0, "[*] [*] [*] [ ]");
                break;
            case 4: lcd595_write(2,0, "[*] [*] [*] [*]");
                break;
            default: lcd595_write(2,0,"erro 2         ");
                break;
            }

        }
        else if(tecla == 'C')
        {
            senha = 0;
            controle = 0;
        }
        else if(tecla == '/')
        {

        }
        else if(tecla == 'x')
        {

        }
        if(tecla != '_')
        {
            sprintf(exibir, "%c", tecla);
            lcd595_write(1,coluna+5,exibir);
            
            coluna++;

        }
        if (controle == 4)
        {
            if (senha == PWD)
            {
                lcd595_clear();
                lcd595_write(1,1,"Abrindo...");
                abrir();
                controle++;
                lcd595_clear();
                lcd595_write(1,1,"Aberto    ");
            }
            else
            {
                erro();
                senha = 0;
                restaura();
            }
        }
        else if (controle == 5)
        {
            tempo--;
            sprintf(exibir,"fechar em %d s ", tempo/5);
            lcd595_write(2,0,exibir);
            if(tempo==0) 
            {
                tempo = 50;
                lcd595_clear();
                lcd595_write(1,1,"Fechando...");
                fechar();
                restaura();
            }
        }


        ESP_LOGI(TAG,"%"PRIu32"  ", adcvalor);
                                                                                                                                                          


        
        //-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - -  -  -  -  -  -  -  -  -  -  Escreva seu só até aqui!!! //
        //________________________________________________________________________________________________________________________________________________________//
        vTaskDelay(200 / portTICK_PERIOD_MS);    // delay mínimo obrigatório, se retirar, pode causar reset do ESP
    }
    
    // caso erro no programa, desliga o módulo ADC
    hcf_adc_limpar();

    /////////////////////////////////////////////////////////////////////////////////////   Fim do ramo principal
    
}
