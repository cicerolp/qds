class NDSQuery{
    constructor(datasetName,group,callback){
	this.queryID      = NDSQuery.getNextID();
	this.datasetName  = datasetName;
	this.aggregations = [];
	this.group        = group;
	this.callback     = callback;
	this.constraints  = [];
    }

    addAggregation(aggregation,aggregationDimension){
	this.aggregations.push({"aggr":aggregation,"dim":aggregationDimension})
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
		//
		var zoom = constraint.payload.zoom;
		//         
		var aux = [Math.floor(lon2tilex(constraint.payload.geometry[1],zoom)),
			   Math.floor(lat2tiley(constraint.payload.geometry[0],zoom)),
			   Math.ceil(lon2tilex(constraint.payload.geometry[3],zoom)),
			   Math.ceil(lat2tiley(constraint.payload.geometry[2],zoom)),
			   zoom];
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

    getAggregationString(){
	var resultStr = "";
	var query = this;
	this.aggregations.forEach(function(aggr){
	    if(aggr.aggr == "quantile"){
		resultStr += ("/aggr=quantile." + aggr.dim + ".(" + query.payload.quantiles.join(":")   + ")/")
	    }
	    else if(aggr.aggr == "inverse_quantile"){
		//return "aggr=inverse.(" + query.payload.inverse_quantile   + ")"
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
