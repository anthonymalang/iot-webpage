'use client'

import { getDatabase, get, ref, onValue, query, orderByKey, limitToLast} from "firebase/database";
import { app } from '../lib/firebase';
import { useEffect, useState } from 'react'

export default function Home() {
  const [currentValue, setCurrentValue] = useState({})
  const [topTen, setTopTen] = useState({})

  useEffect(() => {
    let isMounted = true;

    const db = getDatabase(app);
    const currentDataRef = ref(db, '/data/testuser/current');

    const updateTopTen = async() => {
      const topTenRef = query(ref(db, '/data/testuser/'), orderByKey(), limitToLast(10));
      get(topTenRef).then((snapshot) => {
        if (snapshot.exists()) {
          setTopTen(snapshot.val());
        }
      });
    };
    updateTopTen();
    const intervalId = setInterval(updateTopTen, 2000);
    onValue(currentDataRef, (snapshot) => {
      const data = snapshot.val();
      if (isMounted) {
        setCurrentValue(data);
      }
    });
    return () => {
        isMounted = false;
        clearInterval(intervalId);
    };
  }, []);


  return (
    <div>
      <p>Temperature: {currentValue.temperature}°F</p>
      <p>Humidity:  {currentValue.humidity}%</p>
      <p>Pressure: {currentValue.pressure}hPa</p>
      <p>Altitude: {currentValue.altitude}</p>
      <div>
        <ul>
          {Object.entries(topTen).map(([key, value]) =>
            <li key={key}>
              <p>{key}: Temp: {value.temperature}, Humid: {value.humidity}, Press: {value.pressure}, Alt: {value.altitude}</p>
            </li>
          ).sort((a, b) => { a[0] > b[0]})}
        </ul>
      </div>
    </div>
  );
}
