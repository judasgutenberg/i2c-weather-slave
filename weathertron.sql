 CREATE TABLE `weather_data` (
  `weather_data_id` int(11) NOT NULL AUTO_INCREMENT,
  `recorded` datetime DEFAULT NULL,
  `temperature` decimal(6,3) DEFAULT NULL,
  `pressure` decimal(9,4) DEFAULT NULL,
  `humidity` decimal(5,3) DEFAULT NULL,
  `wind_direction` int(11) DEFAULT NULL,
  `precipitation` int(11) DEFAULT NULL,
  `wind_speed` decimal(8,3) DEFAULT NULL,
  `wind_increment` int(11) DEFAULT NULL,
  `millis` bigint(20) DEFAULT NULL,
  `rain_increment` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`weather_data_id`)
) 