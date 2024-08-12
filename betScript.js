const axios = require('axios');
const cron = require('node-cron');

// The payload for your bet
const betPayload = {"trk":"BLMb","amount":50,"twToken":"05c64bfe-47bd-4687-bc29-a9bd7aab4f18","countrycode":"KE","preference":"254111200811","logMob":111200811}


// Array of User-Agent strings representing different browsers
const userAgents = [
    'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36', // Chrome
    'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0 Safari/605.1.15', // Safari
    'Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko', // Internet Explorer
    'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:78.0) Gecko/20100101 Firefox/78.0', // Firefox
    // Add more User-Agents as needed
  ];

  
// Function to send a bet
const sendBet = async () => {
    // Select a random User-Agent
    const userAgent = userAgents[Math.floor(Math.random() * userAgents.length)];
  
    try {
      const response = await axios.post('https://twbetlionapiv2.betlion.ke/betlion/api/v1.0/truewave/requestawithdrawal', betPayload, {
        headers: {
            'Authorization': 'Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VySWQiOiI2NDM4MmVjYTNkMmRiMTYyNWUxMmIyYTciLCJpYXQiOjE3MDA0MzUzNTYsImV4cCI6MTcwMDQzODk1Nn0.H45yta_ZA2q5Uo92XJJnu1pUs59W_P9PkSKZR9F3OYU',
          'User-Agent': userAgent // Set the User-Agent header
        }
      });
 // Check if the response indicates a successful bet placement
if (response.success) {
      console.log("Success:", response.success.message);
    } else if (response.error) {
      console.error("Error:", response.error.message);
    } else {
      console.log("Unexpected response format.");
    }
  } catch (error) {
    console.error("Error handling the response:", error);
  }
};

// Function to send multiple bets simultaneously
const placeMultipleBets = async (numberOfBets) => {
  const betPromises = Array(numberOfBets).fill().map(_ => sendBet());
  const results = await Promise.all(betPromises);
  console.log("All bets have been placed:", results);
};

// Number of times you want to place the bet
const numberOfBets = 4;

// Schedule the task to run two minutes from now
const twoMinutesLater = new Date();
twoMinutesLater.setMinutes(twoMinutesLater.getMinutes() + 1);
const cronTime = `${twoMinutesLater.getMinutes()} ${twoMinutesLater.getHours()} * * *`;

console.log(`Scheduling the task to run at: ${twoMinutesLater}`);

cron.schedule(cronTime, () => {
  console.log(`Task started at: ${new Date()}`);
  placeMultipleBets(numberOfBets);
}, {
  scheduled: true,
  timezone: "Africa/Nairobi"
});
