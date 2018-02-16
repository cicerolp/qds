var datasets = {
    "green_cabs":{
	"datasetName":"green_tripdata_2013",
	"timeStep":86400,
	"initialTimeConstraint":{"lower":1375315200,"upper":1388534400},
	"temporalDimension":["pickup_datetime"],
	"spatialDimension":["pickup"],
	"categoricalDimension":["passenger_count","payment_type"],
	"payloads":["trip_distance_t","trip_distance_g","total_amount"],
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






