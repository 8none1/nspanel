/*

Defined here:  https://www.metoffice.gov.uk/services/data/datapoint/code-definitions

Value	Description
NA	Not available
0	Clear night
1	Sunny day
2	Partly cloudy (night)
3	Partly cloudy (day)
4	Not used
5	Mist
6	Fog
7	Cloudy
8	Overcast
9	Light rain shower (night)
10	Light rain shower (day)
11	Drizzle
12	Light rain
13	Heavy rain shower (night)
14	Heavy rain shower (day)
15	Heavy rain
16	Sleet shower (night)
17	Sleet shower (day)
18	Sleet
19	Hail shower (night)
20	Hail shower (day)
21	Hail
22	Light snow shower (night)
23	Light snow shower (day)
24	Light snow
25	Heavy snow shower (night)
26	Heavy snow shower (day)
27	Heavy snow
28	Thunder shower (night)
29	Thunder shower (day)
30	Thunder

*/


// These link to the picture ID in Nextion editor
// These are the large icons

const int WEATHER_CODES_LARGE[] = {
209,  // Clear night  - 0
169,  // Sunny day - 1
190,  // Partly cloudy night - 2
149,  // Partly cloudy day 
190, // Not used, so placeholder
269,  // Mist
152, // Fog
149, // Cloudy
145, // Overcast
200, // Light rain night
161, // Light rain day - 10
167, // Drizzle
167, // Light rain
197, // Heavy rain night
158, // Heavy rain day
158, // Heavy rain - 15
201, // Sleet night
162, // Sleet day
162, // Sleet
194, // Hail night
160, // Hail day - 20
160, // Hail
203, // Light snow night
164, // Light snow day
164, // Light snow
203, // Heavy snow night - 25
164, // Heavy snow day
164, // Heavy snow
208, // Thunder night
168, // Thunder shower day - 29
171, // Thunder day - 30
};

const int WEATHER_CODES_LARGE_NIGHT[] = {
209,  // Clear night  - 0
169,  // Sunny day - 1
190,  // Partly cloudy night - 2
149,  // Partly cloudy day 
190, // Not used, so placeholder
269,  // Mist
152, // Fog
190, // Cloudy
190, // Overcast
200, // Light rain night
161, // Light rain day - 10
206, // Drizzle
206, // Light rain
197, // Heavy rain night
158, // Heavy rain day
197, // Heavy rain - 15
201, // Sleet night
162, // Sleet day
201, // Sleet
194, // Hail night
160, // Hail day - 20
194, // Hail
203, // Light snow night
164, // Light snow day
203, // Light snow
203, // Heavy snow night - 25
164, // Heavy snow day
203, // Heavy snow
208, // Thunder night
168, // Thunder shower day - 29
171, // Thunder day - 30
};




const int WEATHER_CODES_SMALL[] = {
84, // 0	Clear night
44, // 1	Sunny day
65, // 2	Partly cloudy (night)
24, // 3	Partly cloudy (day)
24,  // 4	Not used
268, // 5	Mist
268, // 6	Fog
20, // 7	Cloudy.
21, // 8	Overcast
81, // 9	Light rain shower (night)
42, // 10	Light rain shower (day)
42, // 11	Drizzle
42, // 12	Light rain
72, // 13	Heavy rain shower (night)
33, // 14	Heavy rain shower (day)
33, // 15	Heavy rain
76, // 16	Sleet shower (night)
37, // 17	Sleet shower (day)
37, // 18	Sleet
74, // 19	Hail shower (night)
29, // 20	Hail shower (day)
29, // 21	Hail
78, // 22	Light snow shower (night)
39, // 23	Light snow shower (day)
39, // 24	Light snow
78, // 25	Heavy snow shower (night)
39, // 26	Heavy snow shower (day)
39, // 27	Heavy snow
83, // 28	Thunder shower (night)
46, // 29	Thunder shower (day)
46 // 30	Thunder
};

const int WEATHER_CODES_SMALL_NIGHT[] = {
84, // 0	Clear night
44, // 1	Sunny day
65, // 2	Partly cloudy (night)
24, // 3	Partly cloudy (day)
24,  // 4	Not used
268, // 5	Mist
268, // 6	Fog
71, // 7	Cloudy.
71, // 8	Overcast
81, // 9	Light rain shower (night)
42, // 10	Light rain shower (day)
81, // 11	Drizzle
81, // 12	Light rain
72, // 13	Heavy rain shower (night)
33, // 14	Heavy rain shower (day)
72, // 15	Heavy rain
76, // 16	Sleet shower (night)
37, // 17	Sleet shower (day)
76, // 18	Sleet
74, // 19	Hail shower (night)
29, // 20	Hail shower (day)
74, // 21	Hail
78, // 22	Light snow shower (night)
39, // 23	Light snow shower (day)
78, // 24	Light snow
78, // 25	Heavy snow shower (night)
78, // 26	Heavy snow shower (day)
78, // 27	Heavy snow
83, // 28	Thunder shower (night)
46, // 29	Thunder shower (day)
83 // 30	Thunder
};
