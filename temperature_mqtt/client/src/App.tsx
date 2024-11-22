import { useEffect, useState } from "react";
import { client } from "./lib/mqtt";
import { HumChart } from "./components/charts/humChart";
import { TempChart } from "./components/charts/tempChart";

const MQTT_TOPIC_HUMIDITY = "sensor/umidade";
const MQTT_TOPIC_TEMPERATURE = "sensor/temperatura";

export default function App() {
  const [temperature, setTemperature] = useState<number | null>(null);
  const [humidity, setHumidity] = useState<number | null>(null);

  useEffect(() => {
    client.on("message", (topic, message) => {
      const value = parseFloat(message.toString());
      if (topic === MQTT_TOPIC_TEMPERATURE) {
        setTemperature(value);
      } else if (topic === MQTT_TOPIC_HUMIDITY) {
        setHumidity(value);
      }
    });
  }, []);

  return (
    <div className="flex flex-col gap-8 w-full h-screen items-center dark p-10 bg-background">
      <h1 className="text-4xl font-bold text-center text-white">
        DashBoard dos Sensores
      </h1>

      <div className="flex items-center justify-center gap-4">
        <HumChart data={[humidity || 0]} />
        <TempChart data={[temperature || 0]} />
      </div>
    </div>
  );
}