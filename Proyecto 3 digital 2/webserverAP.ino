#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>

// ================= I2C =================
#define addressSlave1 0x18
#define addressSlave2 0x19

#define I2C_SDA 21
#define I2C_SCL 22

// ================= WIFI =================
const char* ssid = "Parqueo-Matic";
const char* password = "12345678";

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

// ================= ESTADOS =================
bool sensores[8] = {0};
bool manual[8]   = {0};
bool usarManual[8] = {0};
bool espacios[8] = {0};

// ================= HTML =================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Parqueo-Matic PRO</title>

<style>
*{margin:0;padding:0;box-sizing:border-box;}

body{
  background: radial-gradient(circle at top,#1a1a2e,#0f0f1a);
  color:#f0f0f0;
  font-family:'Segoe UI';
  padding:20px;
}

.container{max-width:1100px;margin:auto;}

header{
  text-align:center;
  margin-bottom:25px;
}

h1{
  font-size:2.8rem;
  background:linear-gradient(90deg,#4ade80,#22c55e);
  -webkit-background-clip:text;
  -webkit-text-fill-color:transparent;
}

.subtitle{
  color:#aaa;
  margin-top:5px;
}

/* STATS */
.stats{
  display:grid;
  grid-template-columns:repeat(3,1fr);
  gap:15px;
  margin-bottom:25px;
}

.card{
  background:linear-gradient(145deg,#1e1e32,#2a2a45);
  padding:20px;
  border-radius:15px;
  text-align:center;
  box-shadow:0 5px 20px rgba(0,0,0,0.5);
}

.value{
  font-size:32px;
  font-weight:bold;
}

.green{color:#4ade80;}
.red{color:#f87171;}
.yellow{color:#facc15;}

/* GRID */
.grid{
  display:grid;
  grid-template-columns:repeat(4,1fr);
  gap:20px;
}

.space{
  background:linear-gradient(180deg,#2a2a45,#1f1f35);
  border-radius:15px;
  padding:15px;
  text-align:center;
  cursor:pointer;
  transition:0.3s;
  position:relative;
}

.space:hover{
  transform:translateY(-5px);
  box-shadow:0 10px 30px rgba(0,0,0,0.6);
}

.free{
  border:2px solid rgba(74,222,128,0.5);
}

.occupied{
  border:2px solid rgba(248,113,113,0.5);
}

/* CAR */
.car{
  height:90px;
  display:flex;
  align-items:center;
  justify-content:center;
}

/* LED */
.leds{
  display:flex;
  justify-content:center;
  gap:10px;
}

.led{
  width:12px;
  height:12px;
  border-radius:50%;
}

.led-red{
  background:#ef4444;
  box-shadow:0 0 10px #ef4444;
}

.led-green{
  background:#22c55e;
  box-shadow:0 0 10px #22c55e;
}

.led-off{
  background:#333;
}

/* BOTONES */
.buttons{
  margin-top:30px;
  display:flex;
  justify-content:center;
  gap:15px;
}

button{
  padding:12px 20px;
  border:none;
  border-radius:10px;
  cursor:pointer;
  font-weight:bold;
  transition:0.3s;
}

.btn-green{background:#22c55e;}
.btn-red{background:#ef4444;}
.btn-yellow{background:#facc15;color:black;}

button:hover{
  transform:scale(1.05);
}

</style>
</head>

<body>

<div class="container">

<header>
<h1>Parqueo-Matic</h1>
<div class="subtitle">Sistema Inteligente de Estacionamiento</div>
</header>

<div class="stats">
  <div class="card">
    <div class="value green" id="disponibles">0</div>
    <div>Disponibles</div>
  </div>
  <div class="card">
    <div class="value red" id="ocupados">0</div>
    <div>Ocupados</div>
  </div>
  <div class="card">
    <div class="value yellow" id="porcentaje">0%</div>
    <div>Ocupación</div>
  </div>
</div>

<div class="grid" id="grid"></div>

<div class="buttons">
  <button class="btn-green" onclick="vaciar()">Vaciar</button>
  <button class="btn-red" onclick="llenar()">Llenar</button>
  <button class="btn-yellow" onclick="actualizar()">Actualizar</button>
</div>

</div>

<script>

let espacios=[];

function crear(i,ocupado){

return `
<div class="space ${ocupado?'occupied':'free'}" onclick="toggle(${i})">

<div>Espacio ${i+1}</div>

<div class="car">
${ocupado?`
<svg width="60" height="90">
<rect x="5" y="10" width="50" height="70" rx="12" fill="#ef4444"/>
</svg>
`:
`
<svg width="60" height="60">
<circle cx="30" cy="30" r="22" fill="#22c55e"/>
</svg>
`}
</div>

<div class="leds">
<div class="led ${ocupado?'led-red':'led-off'}"></div>
<div class="led ${!ocupado?'led-green':'led-off'}"></div>
</div>

</div>
`;
}

function render(){
  const grid=document.getElementById("grid");
  grid.innerHTML=espacios.map((e,i)=>crear(i,e)).join('');

  let ocupados=espacios.filter(e=>e).length;
  let disponibles=8-ocupados;

  document.getElementById("ocupados").innerText=ocupados;
  document.getElementById("disponibles").innerText=disponibles;
  document.getElementById("porcentaje").innerText=
    Math.round((ocupados/8)*100)+"%";
}

function actualizar(){
 fetch('/status')
 .then(r=>r.json())
 .then(data=>{
   espacios=data.espacios;
   render();
 });
}

function toggle(id){
 fetch('/toggle?id='+id)
 .then(r=>r.json())
 .then(data=>{
   espacios=data.espacios;
   render();
 });
}

function vaciar(){
 fetch('/vaciar')
 .then(r=>r.json())
 .then(data=>{
   espacios=data.espacios;
   render();
 });
}

function llenar(){
 fetch('/llenar')
 .then(r=>r.json())
 .then(data=>{
   espacios=data.espacios;
   render();
 });
}

setInterval(actualizar,1000);
actualizar();

</script>

</body>
</html>
)rawliteral";

// ================= FUNCIONES =================
void actualizarEspaciosFinal() {
  for(int i=0;i<8;i++){
    espacios[i] = usarManual[i] ? manual[i] : sensores[i];
  }
}

void leerSlave(uint8_t direccion, int offset)
{
  Serial.print("\n----- Leyendo Nucleo 0x");
  Serial.print(direccion, HEX);
  Serial.println(" -----");

  Wire.beginTransmission(direccion);
  Wire.write('S');

  uint8_t error = Wire.endTransmission();

  Serial.print("Estado I2C: ");
  if(error == 0) Serial.println("OK");
  else if(error == 1) Serial.println("DATA TOO LONG");
  else if(error == 2) Serial.println("NACK ADDRESS");
  else if(error == 3) Serial.println("NACK DATA");
  else if(error == 4) Serial.println("OTRO ERROR");
  else Serial.println(error);

  if (error != 0) return;

  uint8_t bytesReceived = Wire.requestFrom(direccion, 4);

  Serial.print("Bytes recibidos: ");
  Serial.println(bytesReceived);

  if(bytesReceived == 0){
    Serial.println("⚠ No se recibieron datos");
    return;
  }

  for (int i = 0; i < bytesReceived; i++)
  {
    if (Wire.available())
    {
      uint8_t val = Wire.read();

      Serial.print("Sensor ");
      Serial.print(offset + i + 1);
      Serial.print(" -> RAW: ");
      Serial.print(val);

      sensores[offset + i] = (val > 0);

      Serial.print(" | Estado: ");
      Serial.println(sensores[offset + i] ? "OCUPADO" : "LIBRE");
    }
  }
}

// ================= WEB =================
void handle_Root() {
  server.send(200, "text/html", index_html);
}

String getEspaciosJSON() {
  String json = "{\"espacios\":[";
  for(int i = 0; i < 8; i++) {
    json += espacios[i] ? "true" : "false";
    if(i < 7) json += ",";
  }
  json += "]}";
  return json;
}

void handle_Status() {
  server.send(200, "application/json", getEspaciosJSON());
}

void handle_Toggle() {
  int id = server.arg("id").toInt();
  usarManual[id] = true;
  manual[id] = !manual[id];
  actualizarEspaciosFinal();
  server.send(200,"application/json",getEspaciosJSON());
}

void handle_Vaciar() {
  for(int i=0;i<8;i++){
    usarManual[i]=true;
    manual[i]=false;
  }
  actualizarEspaciosFinal();
  server.send(200,"application/json",getEspaciosJSON());
}

void handle_Llenar() {
  for(int i=0;i<8;i++){
    usarManual[i]=true;
    manual[i]=true;
  }
  actualizarEspaciosFinal();
  server.send(200,"application/json",getEspaciosJSON());
}

uint8_t contarOcupados(){
  uint8_t total = 0;

  for(int i=0;i<8;i++){
    if(espacios[i]) total++;
  }

  return total;
}

// ================= SETUP =================
void setup()
{
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  server.on("/", handle_Root);
  server.on("/status", handle_Status);
  server.on("/toggle", handle_Toggle);
  server.on("/vaciar", handle_Vaciar);
  server.on("/llenar", handle_Llenar);

  server.begin();
}

void enviarEstadoNucleo2()
{
  Wire.beginTransmission(0x19);

  // espacios 5,6,7,8 → índices 4,5,6,7
  Wire.write(espacios[4]);
  Wire.write(espacios[5]);
  Wire.write(espacios[6]);
  Wire.write(espacios[7]);

  uint8_t err = Wire.endTransmission();

  Serial.print("Enviado a 0x19: ");
  Serial.print(espacios[4]);
  Serial.print(espacios[5]);
  Serial.print(espacios[6]);
  Serial.print(espacios[7]);

  Serial.print(" | Error: ");
  Serial.println(err);
}

// ================= LOOP =================
void loop()
{
  server.handleClient();

  leerSlave(addressSlave1, 0);
  leerSlave(addressSlave2, 4);

  for(int i=0;i<8;i++){
    usarManual[i] = false;
  }

  actualizarEspaciosFinal();

  uint8_t total = contarOcupados();

  Wire.beginTransmission(0x18);
  Wire.write(total);
  uint8_t err = Wire.endTransmission();

  Serial.print("Enviado total ocupados: ");
  Serial.print(total);
  Serial.print(" | Error: ");
  Serial.println(err);

  enviarEstadoNucleo2();

  delay(50);
}