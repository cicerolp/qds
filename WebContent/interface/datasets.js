var datasets = {
    "green_cabs":{
	"datasetName":"green_tripdata_2013",
	"timeStep":86400,
	"initialTimeConstraint":{"lower":1375315200,"upper":1388534400},
	"temporalDimension":["pickup_datetime"],
	"spatialDimension":["pickup"],
	"categoricalDimension":["passenger_count","payment_type"],
	"payloads":["trip_distance","total_amount"],
	"payloadsScreenNames":{"trip_distance": "Trip Distance", "total_amount": "Total Fare"},
	"aliases":{"payment_type":{"0":"Credit",
				   "1":"Cash",
				   "2":"No Charge",
				   "3":"Dispute",
				   "4":"Voided trip",
				   "5":"Unknown"},
		   "passenger_count":{
		       "0":"1",
		       "1":"2",
		       "2":"3",
		       "3":"4+",
		   }
		  }
	
    },
    "yellow_cabs":{
	"datasetName":"yellow_tripdata_2011_10",
	"timeStep":14400,
	"initialTimeConstraint":{"lower":1317427200,"upper":1320105600},
	"temporalDimension":["pickup_datetime"],
	"spatialDimension":["pickup"],
	"categoricalDimension":["passenger_count"],
	"payloads":["trip_distance","total_amount"],
	"payloadsScreenNames":{"trip_distance": "Trip Distance", "total_amount": "Total Fare"},
	"inverseQuantileRanges":{"trip_distance":[0,20,1],"total_amount":[0,50,5]},
	"aliases":{"payment_type":{"0":"Credit",
				   "1":"Cash",
				   "2":"No Charge",
				   "3":"Dispute",
				   "4":"Voided trip",
				   "5":"Unknown"},
		   "passenger_count":{
		       "0":"1",
		       "1":"2",
		       "2":"3",
		       "3":"4+",
		   }
		  }
	
    },
    "flights":{
	"datasetName":"on_time_performance_2016",
	"timeStep":86400,
	"initialTimeConstraint":{"lower":1451606400,"upper":1483228800},
	"temporalDimension":["crs_dep_time"],
	"spatialDimension":["origin_airport","dest_airport"],
	"categoricalDimension":["cancelled","diverted,unique_carrier"],
	"payloads":["arr_delay","arr_delay_minutes","actual_elapsed_time","air_time","distance"],
	"payloadsScreenNames":{"arr_delay": "Arrival Delay", "arr_delay_minutes": "Arrival Delay (min)", "actual_elapsed_time":"Actual Elapsed Time", "air_time": "Air Time", "distance": "Distance"},
	"aliases":{"payment_type":{"0":"Credit",
				   "1":"Cash",
				   "2":"No Charge",
				   "3":"Dispute",
				   "4":"Voided trip",
				   "5":"Unknown"},
		   "passenger_count":{
		       "0":"1",
		       "1":"2",
		       "2":"3",
		       "3":"4+",
		   }
		  }
	
    }
}






