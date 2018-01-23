const NDS_MAXCACHE=150;

class NDSInterface{

    constructor(ndsAddress, ndsPort, creationCallBack){
	this.ndsHostAddr = ndsAddress;
	this.ndsPort     = ndsPort;
	//
	this.queryCache = {};
	this.cacheOff = false;
	//
	this.setInternalMaps(creationCallBack);
    }

    setInternalMaps(creationCallBack){	
	//TODO: get schema internal maps
	if(creationCallBack)
	    creationCallBack();
    }
    
    query(q){
	var queryUrl = "http://" + this.ndsHostAddr + ":" + this.ndsPort + "/api/" + q.toString();
	
	var thisObj = {"obj":this,"query":q};
	if(!this.cacheOff && (queryUrl in this.queryCache)){
	    //console.log("***** CACHE HIT");
	    var func = (this.processQueryResponse).bind(thisObj);
	    func(this.queryCache[queryUrl]);	    
	}
	else{
	    //console.log("CACHE FAULT");
	    d3.request(queryUrl)
		.get((this.processQueryResponse).bind(thisObj));
	} 
    }    
   
    processQueryResponse(queryResult){
	if(!this.obj.queryOff){
	    this.obj.queryCache[this.query.toString()] = queryResult;
	}
	var response = JSON.parse(queryResult.response);
	this.query.callback(response,this.query);
    }
}
