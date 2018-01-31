class LatLngTrajectoryLayer{
    constructor(gl, shaderManager, minZoomLevel, maxZoomLevel){

	//
	this.glContext     = gl;
	this.dataInfo      = {};
	
	//
	this.vertexBuffer    = new GLBuffer(this.glContext, this.glContext.ARRAY_BUFFER, "float32");
	this.shaderManager   = shaderManager;
	this.dataInitialized = false;
	this.useColor        = false;
	this.enabled         = true;
	
	//default color scale
	this.colormapTexture = this.glContext.createTexture();
	this.glContext.bindTexture(this.glContext.TEXTURE_2D, this.colormapTexture);
	// 3x1 pixel 1d texture	
	var oneDTextureTexels = new Uint8Array([
	    215,48,39,255,  
	    244,109,67,255, 
	    253,174,97,255, 
	    254,224,144,255,
	    224,243,248,255,
	    171,217,233,255,
	    116,173,209,255,
	    69,117,180,255, 
	]);
	this.glContext.texImage2D(this.glContext.TEXTURE_2D, 0, gl.RGBA, 8, 1, 0, this.glContext.RGBA, this.glContext.UNSIGNED_BYTE,
				  oneDTextureTexels);
    }
    
    setData(trajectories){
	//
	var vertexArray    = [];
	var texCoordArray  = [];
	this.dataInfo = {};
	var trajInfos = []
	var startIndex = 0;
	var totalNumPoints = 0;
	//
	trajectories.forEach(traj=>{	    
	    var numPoints = traj.coords.length
	    trajInfos.push({"objectID": traj.id, "startIndex":startIndex,"endIndex":(startIndex+numPoints-1), "numPoints":numPoints});
	    traj.coords.forEach((pt,i)=>{
		totalNumPoints += 1;
		vertexArray.push(pt.lat);
		vertexArray.push(pt.lng);
		texCoordArray.push((1.0*i)/numPoints);
	    });
	    startIndex += numPoints;
	});
	//
	this.dataInfo["numPoints"] = totalNumPoints;
	this.dataInfo["trajInfos"] = trajInfos;
	//
	this.vertexBuffer.bind();
	this.vertexBuffer.setData(vertexArray.concat(texCoordArray),this.glContext.DYNAMIC_DRAW);
	console.log("Point Vertex Buffer", this.vertexBuffer.getSize());
	//
	this.dataInitialized = true;
    }

    setEnabled(v){
	this.enabled = v;
    }
    
    renderData(){
	//console.log("LatLngPointLayer renderData");

	//
	if(!this.enabled)
	    return;

	//
	var shaderProgram = this.shaderManager.getShader("latLngPointShader");
	shaderProgram.bind();
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
	//
	this.glContext.vertexAttribPointer(
	    shaderProgram.getAttribLocation('aTextureCoordinates'),
	    1,//numComponents,
	    this.glContext.FLOAT,//type,
	    false,//normalize,
	    0,//stride,
	    this.dataInfo.numPoints*2*4);//offset);
	this.glContext.enableVertexAttribArray(shaderProgram.getAttribLocation('aTextureCoordinates'));
	    
	//
	this.glContext.activeTexture(this.glContext.TEXTURE0);
	this.glContext.bindTexture(this.glContext.TEXTURE_2D, this.colormapTexture);
	shaderProgram.setUniform1i('uColormapTexture', 0);
	//
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_MIN_FILTER, this.glContext.LINEAR);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_MAG_FILTER, this.glContext.LINEAR);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_WRAP_S, this.glContext.CLAMP_TO_EDGE);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_WRAP_T, this.glContext.CLAMP_TO_EDGE);

	    
	//
	if(this.dataInitialized && this.dataInfo.numPoints > 0){
	    this.glContext.enable(this.glContext.BLEND);
	    this.glContext.blendFunc(this.glContext.SRC_ALPHA, this.glContext.ONE_MINUES_SRC_ALPHA);

	    var lineWidth = this.glContext.getParameter(this.glContext.LINE_WIDTH);
	    this.glContext.lineWidth(10);

	    this.dataInfo.trajInfos.forEach(trajInfo=>{
		this.glContext.drawArrays(this.glContext.LINE_STRIP,0,this.dataInfo.numPoints);
 		this.glContext.drawArrays(this.glContext.POINTS,trajInfo.startIndex,trajInfo.numPoints);
	    });
	    this.glContext.lineWidth(lineWidth);
	    
	    
	    // this.glContext.drawArrays(this.glContext.LINE_STRIP,0,this.dataInfo.numPoints);

	    // this.glContext.drawArrays(this.glContext.POINTS,0,this.dataInfo.numPoints);

	    
	    this.glContext.disable(this.glContext.BLEND);
	}

	this.glContext.bindTexture(this.glContext.TEXTURE_2D, null);
	shaderProgram.release();
    }
}
