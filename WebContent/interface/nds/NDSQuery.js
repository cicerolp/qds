class NDSQuery{
    constructor(datasetName,aggregation,group,callback){
	this.queryID     = NDSQuery.getNextID();
	this.datasetName = datasetName;
	this.aggregation = aggregation;
	this.group       = group;
	this.callback    = callback;
	this.constraints = [];
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
	if(this.aggregation == "quantile"){
	    return "aggr=quantile.(" + this.payload.quantiles.join(":")   + ")"
	}
	else if(this.aggregation == "inverse_quantile"){
	    return "aggr=inverse.(" + this.payload.inverse_quantile   + ")"
	}
	else if(this.aggregation == "count"){
	    return  "aggr=count";
	}
	else{
	    return undefined;
	}
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
