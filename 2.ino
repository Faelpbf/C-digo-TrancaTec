#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
// Configurações do LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
//rele
const int rele = A1;
//buzzer
int b = 8;
//LEDs
const int ledf = 9, ledt = 10;
// Configurações da senha
const int Tamanho_Senha = 6;
String senha_mestre = "AAAAAA"; //sujeita a troca
int tentativa = 0;  // Variável global para contar as tentativas
// Configuração do teclado matricial
const int linha1 = 7, linha2 = 6, linha3 = 5, linha4 = 4;
const int coluna1 = 3, coluna2 = 2, coluna3 = 11, coluna4 = 12;
const int linhas = 4, colunas = 4;
//Mapeamento do teclado
char mapTeclas[4][4] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte pinos_linha[4] = { linha1, linha2, linha3, linha4 };
byte pinos_coluna[4] = { coluna1, coluna2, coluna3, coluna4 };

Keypad teclado = Keypad(makeKeymap(mapTeclas), pinos_linha, pinos_coluna, linhas, colunas);

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(rele, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(ledf, OUTPUT);
  pinMode(ledt, OUTPUT);

  beep_inicial(); 
}

void loop() {
  digitalWrite(ledf, HIGH);
  char tecla = teclado.getKey();  
  if (tecla) {
    beep(); 
  }

//Verificação inicial para ver se existe uma senha cadastrada ou não
  if (V_PrimeiroAcesso()) {
    config_Senha();
  } else {
    senha_acesso();
  }
}

bool V_PrimeiroAcesso() {
  for (int i = 0; i < EEPROM.length(); i++) {
    if (EEPROM.read(i) != 0xFF) {
      return false;
    }
  }
  return true;
}

void config_Senha() {
  lcd.setCursor(0, 0);
  lcd.print("Cadastre a senha:");

  int cursor = 0;
  String nova_senha = "";
  char tecla;

  while (nova_senha.length() < Tamanho_Senha) {
    tecla = teclado.getKey();
    if (tecla) {
      beep();
      nova_senha += tecla;
      lcd.setCursor(cursor, 1);
      lcd.print('*');
      cursor++;
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirme a senha:");
  String confirmar_senha = "";
  cursor = 0;

  while (confirmar_senha.length() < Tamanho_Senha) {
    tecla = teclado.getKey();
    if (tecla) {
      beep();
      confirmar_senha += tecla;
      lcd.setCursor(cursor, 1);
      lcd.print('*');
      cursor++;
    }
  }

  lcd.clear();
  if (nova_senha == confirmar_senha) {
    //armazena a senha criada na eeprom
    for (int i = 0; i < Tamanho_Senha; i++) {
      EEPROM.write(i, nova_senha[i]);
    }

    lcd.setCursor(0, 0);
    digitalWrite(ledf, LOW);
    digitalWrite(ledt, HIGH);
    lcd.print("Senha Salva!");
    delay(3000);
    digitalWrite(ledt, LOW);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Senha nao bate!");
    delay(3000);
  }

  lcd.clear();
}

void senha_acesso() { 
  lcd.setCursor(0, 0);
  lcd.print("Senha:");

  int cursor = 0;
  String senha_digitada = "";
  char tecla;

  while (senha_digitada.length() < Tamanho_Senha) {
    tecla = teclado.getKey();
    if (tecla) {
      beep();
      senha_digitada += tecla;
      lcd.setCursor(cursor, 1);
      lcd.print('*');
      cursor++;
    }
  }

  // armazenana a senha da EEPROM em uma variável
  String senha_armazenada = "";
  for (int i = 0; i < Tamanho_Senha; i++) {
    senha_armazenada += char(EEPROM.read(i));
  }

  lcd.clear();

  // Verificações de senha
  if (senha_digitada == senha_armazenada) {
    lcd.setCursor(0, 0);
    digitalWrite(ledf, LOW);
    digitalWrite(ledt, HIGH);
    lcd.print("Acesso Liberado!");
    beep_acesso_liberado();
    digitalWrite(rele, HIGH);
    delay(5000);
    digitalWrite(rele, LOW);
    digitalWrite(ledt, LOW);
    tentativa = 0;  // Zera o contador após sucesso
  } else if (senha_digitada == senha_mestre) {
    painel_adm();
    tentativa = 0;  // Zera o contador no painel
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Senha Incorreta!");
    tentativa++;
    beep_acesso_negado();
    delay(2000);
  }

  if (tentativa >= 6) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BLOQUEADO!");
    lcd.setCursor(0, 1);
    lcd.print("Aguarde 20s...");
    tone(b, 1000);
    delay(20000);
    noTone(b);
    tentativa = 0;  // Reseta tentativas após bloqueio
  }

  lcd.clear();
}

void painel_adm() {
  lcd.clear();  
  lcd.setCursor(0, 0);
  lcd.print("Painel do Admin");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1-Trocar a senha");
  lcd.setCursor(0, 1);
  lcd.print("2-Resetar");

  // Espera o usuário pressionar uma tecla válida
  char tecla;
  while (true) {
    tecla = teclado.getKey();  // Lê a tecla pressionada
    if (tecla == '1') {
      troca_senha();
      break;  // Sai do loop
    } else if (tecla == '2') {
      limpa_EEPROM();
      break;  // Sai do loop
    }
  }

  lcd.clear();
}

void troca_senha() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nova senha:");

  int cursor = 0;
  String nova_senha = "";
  char tecla;

  
  while (nova_senha.length() < Tamanho_Senha) {
    tecla = teclado.getKey();
    if (tecla) {
      beep();
      nova_senha += tecla;
      lcd.setCursor(cursor, 1);
      lcd.print(tecla); // Mostra os números digitados
      cursor++;
    }
  }

  // Confirmar a nova senha
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirme senha:");
  String confirmar_senha = "";
  cursor = 0;

  while (confirmar_senha.length() < Tamanho_Senha) {
    tecla = teclado.getKey();
    if (tecla) {
      beep();
      confirmar_senha += tecla;
      lcd.setCursor(cursor, 1);
      lcd.print('*'); // Mostra os asteriscos durante a confirmação
      cursor++;
    }
  }

  // Verifica se a senha foi confirmada corretamente
  lcd.clear();
  if (nova_senha == confirmar_senha) {
    // Salva a nova senha na EEPROM
    for (int i = 0; i < Tamanho_Senha; i++) {
      EEPROM.write(i, nova_senha[i]);
    }
    lcd.setCursor(0, 0);
    lcd.print("Senha Alterada!");
    Serial.println("Senha alterada com sucesso!");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Senha nao bate!");
    Serial.println("Erro: Senhas nao conferem.");
  }

  delay(3000);
  lcd.clear();
}

void limpa_EEPROM() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Resetando...");

  Serial.println("Limpando EEPROM...");

  // Loop para limpar todos os endereços da EEPROM
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0xFF);  // Enche a eprrom de lixo
  }

  Serial.println("EEPROM limpa!");
}



// beeps

void beep() {
  tone(b, 1000, 100);  // Frequência de 1000 Hz por 100 ms
}

void beep_inicial() {
  tone(b, 2000, 200);  // Frequência de 2000 Hz por 200 ms
  delay(200);
}

void beep_acesso_liberado(){
    tone(b, 1000, 200);
    delay(300);
    tone(b, 500, 400);
}

void beep_acesso_negado(){
  for (int i = 0; i < 2; i++) {
      tone(b, 400, 100);
      delay(200);
    }
}
