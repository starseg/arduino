// mqttClient.ts
import mqtt from "mqtt";

const MQTT_URL = "wss://37f55325275a47b49a6030354a67d6b0.s1.eu.hivemq.cloud:8884/mqtt";
const MQTT_USER = "dutra";
const MQTT_PASSWORD = "Fe070104@";

const MQTT_TOPIC_HUMIDITY = "sensor/umidade";           // Tópico de umidade
const MQTT_TOPIC_TEMPERATURE = "sensor/temperatura";         // Tópico de temperatura

// Cria a instância do cliente MQTT
export const client = mqtt.connect(MQTT_URL, {
  username: MQTT_USER,
  password: MQTT_PASSWORD,
});

client.on("connect", () => {
  console.log("Conectado ao MQTT via WebSocket");
  client.subscribe(MQTT_TOPIC_HUMIDITY);
  client.subscribe(MQTT_TOPIC_TEMPERATURE);
  console.log("Subscrito aos tópicos de umidade e temperatura");
});
