export default async function Home() {
  const res = await fetch(
    'https://iot-webpage-34ee1-default-rtdb.firebaseio.com/testuser.json'
  );

  const data = await res.json();

  const readings = Object.entries(data || {}).map(([id, values]) => ({
    id,
    ...values,
  }));

  return (
    <ul>
      {readings.map((reading) => (
        <li key={reading.id}>
          Temperature: {reading.temperature}°F,
          Humidity: {reading.humidity}%,
          Pressure: {reading.pressure} hPa,
          Gas: {reading.gas},
          Altitude: {reading.altitude}
        </li>
      ))}
    </ul>
  );
}