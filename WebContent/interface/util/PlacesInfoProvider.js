class PlaceInfoProvider{
    constructor(){
	this.protocol = "http://"
	this.hostName = "10.0.100.237"
	this.portNumber = "5000"
	this.queryCache = {};
	this.cacheOff = false;
    }

    getInfo(placeID,callback){
	var url = this.protocol + this.hostName + ":" + this.portNumber + "/place/" + placeID;
	if(!this.cacheOff && (placeID in this.queryCache)){
	    var result = this.queryCache[placeID];
	    result.query = placeID;
	    callback(result);	    
	}
	else{
	    d3.request(url)
		.header("X-Requested-With", "XMLHttpRequest")
		.header("Accept", "application/json")
		.get((this.processResult).bind({"callback":callback,"obj":this,"query":placeID}));
	}
    }

    getInfos(ids,callback){
	var that = this;

	var idSet = new Set(ids);
	var results = {};
	
	ids.forEach(d=>that.getInfo(d,function(result){
	    if("id" in result){
		idSet.delete(result["id"])
		results[result["id"]] = result;
	    }
	    else{
		idSet.delete(result.query);
		results[result.query] = {};
	    }
	    if(idSet.size == 0)
		callback(results);
	}));
    }

    getNearbyPlaces(lat,lng,radius,limit,callback){
	var url = "http://localhost:5000/nearby/" + lat +"/" + lng + "/" + radius  + "/" + limit;
	//console.log(url);
	if(!this.cacheOff && url in this.queryCache){
	    callback(this.queryCache[url]);
	}
	else{
	    d3.request(url).get(
		(function(d){
		    this.queryCache[url] = d;
		    callback(d);
		}).bind(this));
	}
    }
    
    processResult(queryResult){
	var result = JSON.parse(queryResult.response);
	if(!this.obj.queryOff){
	    this.obj.queryCache[this.query.toString()] = result;
	    result.query = this.query;
	}
	this.callback(result);
    }
    
    
}
