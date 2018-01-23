class GLBuffer{
    constructor(gl, target, varType){
	this.glContext = gl;
	this.bufferID  = this.glContext.createBuffer();
	this.target    = target;
	this.size      = 0;
	this.varType   = varType;
    }

    setData(newData,usage){
	var serializedData = undefined;
	if(this.varType == "float32")
	    serializedData = new Float32Array(newData);
	else if(this.varType == "uint16")
	    serializedData = new Uint16Array(newData);
	else{
	    debugger
	    alert("Buffer type error!!!!!");
	    return undefined
	}

	var newSize = serializedData.byteLength;

	this.glContext.bufferData(this.target,serializedData,usage);	
	this.size = serializedData.byteLength;
    }

    setSubData(newData,byteOffset){
	debugger
	var serializedData = undefined;
	if(this.varType == "float32")
	    serializedData = new Float32Array(newData);
	else if(this.varType == "uint16")
	    serializedData = new Uint16Array(newData);
	else{
	    debugger
	    alert("Buffer type error!!!!!");
	    return undefined
	}

	//var newSize = serializedData.byteLength;
	//this.size = serializedData.byteLength;
	this.glContext.bufferSubData(this.target,byteOffset,serializedData);	
    }
    
    bind(){
	this.glContext.bindBuffer(this.target,this.bufferID);
    }

    release(){
	this.glContext.bindBuffer(this.target,null);
    }
    
    getSize(){
	return this.size;
    }

    dispose(){
	this.glContext.deleteBuffer(this.bufferID);
    }
}
