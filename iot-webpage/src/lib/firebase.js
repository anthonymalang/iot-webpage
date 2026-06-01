import { initializeApp } from 'firebase/app';

// TODO: Replace the following with your app's Firebase configuration
const firebaseConfig = {
  apiKey: process.env.NEXT_PUBLIC_FIREBASE_API_KEY,
  databaseURL: process.env.NEXT_PUBLIC_FIREBASE_DATABASE_URL
};

const app = initializeApp(firebaseConfig);

export { app };
