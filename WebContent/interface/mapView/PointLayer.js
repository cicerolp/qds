class PointLayer{
    constructor(gl, shaderManager, minZoomLevel, maxZoomLevel){
	//
	this.glContext     = gl;
	this.dataInfo      = {};
	
	//
	this.vertexBuffer    = new GLBuffer(this.glContext, this.glContext.ARRAY_BUFFER, "float32");
	this.shaderManager   = shaderManager;
	this.dataInitialized = false;
	this.useColor        = false;
    }

    //
    loadJSONFile(dataFile){
	//TODO
	alert("ERROR Point Layer: Using function loadJASONFile not implemented!");
	return undefined;
    }
    
    setData(dataInfo,vertexArray,colorArray){
	if(colorArray == undefined){
	    colorArray    = [];
	    this.useColor = false;
	}
	else{
	    this.useColor = true;
	}
	
	//
	this.dataInfo = dataInfo;
	//
	this.vertexBuffer.bind();
	this.vertexBuffer.setData(vertexArray.concat(colorArray),this.glContext.STATIC_DRAW);
	console.log("Point Vertex Buffer", this.vertexBuffer.getSize());
	//
	this.dataInitialized = true;
    }
    
    renderData(){
	//
	const numComponents = 2;
	const type = this.glContext.FLOAT;
	const normalize = false;
	const stride = 0;
	const offset = 0;
	
	var shaderProgram = undefined;	
	if(this.useColor){
	    shaderProgram = undefined;
	    alert("PointLayer: Not IMPLEMENTED!");
	    //
	    this.vertexBuffer.bind();
	    //
	    this.glContext.vertexAttribPointer(
		shaderProgram.getAttribLocation('aVertexPosition'),
		2,//numComponents,
		this.glContext.FLOAT,//type,
		false,//normalize,
		0,//stride,
		0);//offset);
	    glContext.enableVertexAttribArray(shaderProgram.getAttribLocation('aVertexPosition'));

	    //
	    
	    this.glContext.vertexAttribPointer(
		shaderProgram.getAttribLocation('aVertexColor'),
		4,//numComponents,
		this.glContext.FLOAT,//type,
		false,//normalize,
		0,//stride,
		2*4*this.dataInfo.numPoints);//2 coordinates * 4 bytes for each float * numPoints
	    this.glContext.enableVertexAttribArray(shaderProgram.getAttribLocation('aVertexColor'));
	}
	else{
	    shaderProgram = this.shaderManager.getShader("pointShader");
	    shaderProgram.bind();
	    //
	    this.vertexBuffer.bind();
	    //
	    this.glContext.vertexAttribPointer(
		shaderProgram.getAttribLocation('aVertexPosition'),
		2,//numComponents,
		this.glContext.FLOAT,//type,
		false,//normalize,
		0,//stride,
		0);//offset);
	    this.glContext.enableVertexAttribArray(shaderProgram.getAttribLocation('aVertexPosition'));
	}
	    
	//
	if(this.dataInitialized){
	    this.glContext.drawArrays(this.glContext.POINTS,0,this.dataInfo.numPoints);
	}
	
	shaderProgram.release();
    }
}
