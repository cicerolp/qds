class NanocubeInterface{

    constructor(nanocubeAddress, nanocubePort, baseDateStr){
	this.nanocubeHostAddr = nanocubeAddress;
	this.nanocubePort     = nanocubePort;
	this.constraints      = {};
	this.geoDiveLevel     = 15;
	this.baseDateStr = baseDateStr;
	this.valueDimension = "count";
	this.queryCallBacks = {};
	//get type mapping
	// d3.queue()
	//     .defer(this.getAppNameMap)
	//     .awaitAll(function(){console.log("Done loading Nanocube");});
	//TODO: Have to guarantee that this is done before starting using the interface
	this.getAppNameMap();
    }

    setValueDimension(v){
	this.valueDimension = v;
    }
    
    getAppNameMap(){
	
	var schemaQuery = this.nanocubeHostAddr + ":" + this.nanocubePort + "/schema()";
	var that = this;
	that.dimensionAliasMaps = {};
	d3.request(schemaQuery)
	    .get(function(queryResult){
		var response = JSON.parse(queryResult.response)[0];
		var indexDimensions = response["index_dimensions"];
		indexDimensions.forEach(function(d){
		    var name = d.name;
		    if(d.hint == "categorical"){
			//
			that.dimensionAliasMaps[name] = {};
			that.dimensionAliasMaps[name]["mapPathToAlias"] = d.aliases;
			that.dimensionAliasMaps[name]["mapAliasToPath"] = {};
			for(var key in d.aliases){
			    var newKey = key.split(":").join(",");
			    that.dimensionAliasMaps[name]["mapAliasToPath"][d.aliases[key]] = newKey;
			}
		    }
		});
		
		console.log("Done Prepearing Nanocube");
	    });
    }

    
    registerWidget(name){
	alert("THIS SHOULD NOT BE CALLED");
	this.widgets[name] = "";//empy constraint
    }

    setConstraint(dimName,constraint){
	if(constraint == null)
	    delete this.constraints[dimName];
	else{
	    this.constraints[dimName] = constraint;
	}
    }

    getBaseTime(){
	var url = this.nanocubeHostAddr + ":" + this.nanocubePort + "/schema()";
	d3.request(url)
	    .get((this.processQueryResponse).bind({"obj":this,"query":"BASE_TIME"}));
    }
    
    getQueryString(queryID){
	//TODO: build up the right constraint constraint
	switch(queryID){
	case "GEO":
	    return ".b('location',dive(" + this.geoDiveLevel + "))";
	    break;
	case "GLOBAL_TIME":
	    //retrieve from nanocube
	    return ".b('time',timeseries('" + this.baseDateStr + " 00:00:00',3600,720,3600))";	
	    break;
	case "APP":
	    //retrieve from nanocube
	    return ".b('type',dive(3),'name')";	
	    break;
	case "LABELS":
	    //retrieve from nanocube
	    return ".b('label',dive(3),'name')";	
	    break;
	case "NUM_LABELS":
	    //retrieve from nanocube
	    return ".b('numLabels',dive(1),'name')";	
	    break;
	case "NUM_PLACES":
	    //retrieve from nanocube
	    return ".b('numPlaces',dive(1),'name')";	
	    break;
	case "RELIABILITY":
	    //retrieve from nanocube
	    return ".b('reliability',dive(1),'name')";	
	    break;
	case "RELIABILITY_ALL":
	    //retrieve from nanocube
	    return ".b('numLabels',dive(1),'name').b('numPlaces',dive(1),'name').b('reliability',dive(1),'name').b('label',dive(3),'name')";	
	    break;
	case "REQ_GEO":
	    return ".b('location',dive(" + this.geoDiveLevel + ")).b('type',dive(1),'name')";
	    break;
	default:
	    alert("invalid queryID " + queryID + "!");
	    return undefined;
	}
    }

    setGeographicalDiveLevel(level){
	this.geoDiveLevel = level;
    }

    getConstraints(){
	var strConstraints = "";
	for(var key in this.constraints){
	    if(key == "TIME"){
		var interval = this.constraints[key];
		var nanoConst = ".b('time',interval("+ interval[0] + "," + interval[1] +"))";
		strConstraints += nanoConst;
	    }
	    else if(key in this.dimensionAliasMaps){
		var mapAliasToPath = this.dimensionAliasMaps[key].mapAliasToPath;
		var appConstraint = this.constraints[key];
		if(appConstraint.size == 0)
		    continue;
		//
		var strList = [];
		appConstraint.forEach((function(d){
		    if(d == "others"){
			//TODO
		    }
		    else
			strList.push("p(" + mapAliasToPath[d] + ")")
		}).bind(this));
		var constStr = ".b('"+key+"',pathagg(" + strList.join(",") + ")" + ")";
		strConstraints += constStr;

	    }
	}
	return strConstraints;
    }
    
    queryNanocube(queryID){
	
	var url = this.nanocubeHostAddr + ":" + this.nanocubePort + "/";

	//add constraints
	var constraints = this.getConstraints();
	
	//
	var queryString = this.getQueryString(queryID)

	var fullString = url + "q(visits" + constraints + queryString + ")";
	console.log(fullString);
	
	
	//
	d3.request(fullString)
	    .get((this.processQueryResponse).bind({"obj":this,"query":queryID}));
    }

    getNanocubeSchema(){
	// var url = this.nanocubeHostAddr + ":" + this.nanocubePort + "/schema()";
	// d3.request(url)
	//     .get((this.processQueryResponse).bind({"obj":this,"query":"BASE_TIME"}));
    }

    processSchemaResponse(){
	console.log(queryResult.response);
    }

    getTargetValueDimension(measureColumns,dimName){
	var result = undefined;
	measureColumns.forEach(function(d){
	    if(d.name == dimName)
		result = d;
	});
	return result;
    }
    processQueryResponse(queryResult){
	var response = JSON.parse(queryResult.response)[0]; //assuming one table
	
	//process time
	if(this.query == "GLOBAL_TIME"){
	    var columnInfo = response["index_columns"][0]["values"];

	    //if not conts, return averages
	    var counts = this.obj.getTargetValueDimension(response["measure_columns"],"count")["values"]
	    var values = undefined;
	    if(this.obj.valueDimension == "count")
		values = counts;
	    else{
		var targetValueSums = this.obj.getTargetValueDimension(response["measure_columns"],this.obj.valueDimension)["values"];	
		values = targetValueSums.map(function(d,i){
		    return d/counts[i];
		});
	    }

	    //
	    var zippedArrays = columnInfo.map(function (e, i) {return [e, values[i]];});
	    this.obj.timeCallBack([zippedArrays],["black"]);	
	}
	else if(this.query == "GEO"){
	    //process location
	    var columnInfo      = response["index_columns"][0];
	    var numValuesPerRow = columnInfo["values_per_row"];
	    var values          = columnInfo["values"];
	    var numRows         = values.length/numValuesPerRow;
	    var treePaths       = [];

	    for(var i = 0 ; i < numRows ; ++i){
		var path = "";
		for(var j = 0 ; j < numValuesPerRow ; ++j)
		    path += values[numValuesPerRow*i+j];
		treePaths.push(path);
	    }
	    
	    //
	    var counts = this.obj.getTargetValueDimension(response["measure_columns"],"count")["values"]
	    var values = undefined;
	    if(this.obj.valueDimension == "count")
		values = counts;
	    else{
		var targetValueSums = this.obj.getTargetValueDimension(response["measure_columns"],this.obj.valueDimension)["values"];	
		values = targetValueSums.map(function(d,i){
		    return d/counts[i];
		});
	    }

	    
	    this.obj.geoCallBack(treePaths,values);
	}
	else if(this.query == "REQ_GEO"){
	    //process location
	    var columnInfo      = response["index_columns"][0];
	    var numValuesPerRow = columnInfo["values_per_row"];
	    var locationValues  = columnInfo["values"];
	    var typeValues      = response["index_columns"][1]["values"];
	    var numRows         = locationValues.length/numValuesPerRow;
	    var treePaths       = [];
	    var pathSet         = {};
	    var counts = this.obj.getTargetValueDimension(response["measure_columns"],"count")["values"]
	    
	    for(var i = 0 ; i < numRows ; ++i){
		var reqType = typeValues[i];
		var path = "";
		for(var j = 0 ; j < numValuesPerRow ; ++j)
		    path += locationValues[numValuesPerRow*i+j];
		if(!(path in pathSet)){
		    pathSet[path] = {};
		}
		pathSet[path][reqType] = counts[i];
	    }
	    this.obj.queryCallBacks[this.query](this.query,pathSet);
	}
	else if(this.query == "BASE_TIME"){
	    //todo process string
	    return "2017-01-01";
	}
	else if(this.query == "APP"){
	    var columnInfo = response["index_columns"][0]["values"];
	    //
	    var values     = response["measure_columns"][0]["values"];	
	    //
	    var result      = columnInfo.map(function (e, i) {return {"appName":e, "numVisits":values[i]};});
	    this.obj.appCallBack(result);
	}
	else if(this.query == "RELIABILITY" ||this.query == "NUM_LABELS" || this.query == "LABELS" || this.query == "NUM_PLACES"){
	    //debugger
	    console.log("response",this.query);
	    var columnInfo = response["index_columns"][0]["values"];
	    var values     = response["measure_columns"][0]["values"];	
	    //
	    var result      = columnInfo.map(function (e, i) {return {"key":e, "value":values[i]};});
	    
	    this.obj.queryCallBacks[this.query](this.query,result);
	}
	else if(this.query == "RELIABILITY_ALL"){
	    //debugger
	    var indexColumns = response["index_columns"];
	    var incrementPerDimension = indexColumns.map(function(d){return +d.values_per_row;})
	    var responseSize = indexColumns[0].values.length / incrementPerDimension[0];
	    var counts = response["measure_columns"][0].values; //assuming count is the first value dimension
	    //
	    
	    //pick default permutation for now
	    for(var i = 0 ; i < responseSize ; ++i){
		var singleResponse = indexColumns.map(function(d,dimensionIndex){
		    return d.values[i*incrementPerDimension[dimensionIndex]];
		});
		var value = +counts[i];
		//
	    }
	}
    }
}
