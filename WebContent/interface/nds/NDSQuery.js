class NDSQuery{
    constructor(datasetName,group,callback){
	this.queryID      = NDSQuery.getNextID();
	this.datasetName  = datasetName;
	this.aggregations = [];
	this.group        = group;
	this.callback     = callback;
	this.constraints  = [];
	this.threshold    = undefined;
    }

    addAggregation(aggregation,aggregationDimension){
	this.aggregations.push({"aggr":aggregation,"dim":aggregationDimension})
    }

    setThreshold(v){
	this.threshold = v;
    }
    
    //TODO: Remove parameter type and make it a member of constraint in all cases
    addConstraint(type,dimensionId,payload){
	var newConstraint = {"type":type, "dimensionId":dimensionId,"payload":payload};
	this.constraints.push(newConstraint);
    }
    
    getConstraintString(){
	var strConstraints = "";
	
	for(var key in this.constraints){
	    var constraint = this.constraints[key];
	    var dimension = constraint.dimension;
	    var myStr = "";
	    
	    if(constraint.type == "categorical"){
		myStr = ("const=" + constraint["dimensionId"] + ".values.(" + constraint.payload.values.join(":") + ")/");
	    }
	    if(constraint.type == "tile"){
		myStr = ("const=" + constraint["dimensionId"] + ".tile.(" + constraint.payload.x + ":" + constraint.payload.y + ":" + constraint.payload.z + ":" + constraint.payload.resolution  + ")/");
	    }
	    if(constraint.type == "time_interval"){	
		myStr = ("const=" + constraint["dimensionId"] + ".interval.(" + constraint.payload.lower+ ":" + constraint.payload.upper + ")/");		
	    }
	    if(constraint.type == "region"){
		// //
		// var lat0 = b._northEast.lat;
		// var lon0 = b._southWest.lng;
		// var lat1 = b._southWest.lat;
		// var lon1 = b._northEast.lng;
		//
		var lat0 = constraint.payload.geometry[0];
		var lon0 = constraint.payload.geometry[1];
		var lat1 = constraint.payload.geometry[2];
		var lon1 = constraint.payload.geometry[3];

		var z = constraint.payload.zoom;

		var x0 = roundtile(lon2tilex(lon0, z), z);
		var x1 = roundtile(lon2tilex(lon1, z), z);

		if (x0 > x1) {
                    x0 = 0;
                    x1 = Math.pow(2, z);
		}

		//
		var zoom = constraint.payload.zoom;
		//         
		var aux = [x0,roundtile(lat2tiley(lat0, z), z),
			   x1,roundtile(lat2tiley(lat1, z), z),z];
		//
		//const=0.region.(0:0:1:1:0)
		myStr = ("const=" + constraint["dimensionId"] + ".region.(" + aux.join(":")  + ")/");		
	    }	    

	    strConstraints += myStr;
	}
	return strConstraints;
    }

    setPayload(p){
	this.payload = p;
    }
    
    getGroupString(){
	if(this.group != undefined){
	    return "group="+this.group;
	}
	else 
	    return "";
    }

    getThresholdString(){
	if(this.threshold == undefined)
	    return "";
	else
	    return "/threshold=" + this.threshold + "/"
    }
    
    getAggregationString(){
	var resultStr = "";
	var query = this;
	this.aggregations.forEach(function(aggr){
	    if(aggr.aggr == "quantile"){
		resultStr += ("/aggr=quantile." + aggr.dim + ".(" + query.payload.quantiles.join(":")   + ")/")
	    }
	    else if(aggr.aggr == "average"){
		resultStr += ("/aggr=average." + aggr.dim + "/")
	    }
	    else if(aggr.aggr == "inverse_quantile"){
		resultStr += ("/aggr=inverse." + aggr.dim + ".(" + query.payload.inverse_quantile   + ")/")
	    }
	    else if(aggr.aggr == "count"){
		resultStr += "/aggr=count/";
	    }
	    else{
		//return undefined;
	    }

	});
	
	return resultStr;
    }

    getDatasetString(){
	return "dataset=" + this.datasetName;
    }
    
    toString(){
	var components = ["query",this.getDatasetString(),this.getAggregationString(),this.getConstraintString(),this.getGroupString()];
	components = components.filter(d=> (d != ""));
	return components.join("/");
    }
}

NDSQuery.nextID = 0;
NDSQuery.getNextID = function(){
    var id = NDSQuery.nextID;
    NDSQuery.nextID += 1;
    return id;
}
