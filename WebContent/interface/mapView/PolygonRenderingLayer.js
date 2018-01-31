class PolygonRenderingLayer{
    constructor(gl, minZoomLevel, maxZoomLevel){
	//
	this.glContext = gl;
	this.dataInfo = {};
	
	//
	this.vertexBuffer = new GLBuffer(this.glContext, this.glContext.ARRAY_BUFFER, "float32");
	this.indexBuffer  = new GLBuffer(this.glContext, this.glContext.ELEMENT_ARRAY_BUFFER, "uint16");

	//
	this.shaderProgram = undefined

	//
	this.dataInitialized = false;
    }

    //
    loadJSONFile(dataFile){
	var widget = this;
	d3.json(dataFile,function(dataOBJ){
	    var data = dataOBJ["data"];
	    var numObjects   = data.length;
	    var vertexCoords = [];
	    var faceIndices  = [];
	    var offset       = 0;
	    
	    for(var _index in data){
		var elt = data[_index];
		var numOfVertices = elt.geometry.length;
		//
		elt.geometry.forEach(function(v){
		    vertexCoords.push(+v[0]);
		    vertexCoords.push(+v[1]);
		})
		//
		elt.indices.forEach(function(face){
		    faceIndices.push(+face[0] + offset);
		    faceIndices.push(+face[1] + offset);
		    faceIndices.push(+face[2] + offset);
		});
		//
		widget.dataInfo[elt.id] = {"name":elt.name,"minIndexValue":offset,"maxIndexValue":(offset+numOfVertices)};
		//
		offset += numOfVertices;
	    }
	    
	    //
	    widget.indexBuffer.bind();
	    widget.indexBuffer.setData(faceIndices, widget.glContext.STATIC_DRAW);
	    console.log("Index Buffer", widget.indexBuffer.getSize());
	    
	    widget.vertexBuffer.bind();
	    widget.vertexBuffer.setData(vertexCoords, widget.glContext.STATIC_DRAW);
	    console.log("Vertex Buffer", widget.vertexBuffer.getSize());
	    //
	    widget.dataInitialized = true;
	})
    }

    setShaderProgram(shaderProgram){
	this.shaderProgram = shaderProgram
    }
    
    //vertex array is a collection of the form [ [v1_x,v1_y], [v2_x,v2_y], ..., [vN_x,vN_y]]
    //index array is a collection of the form [ [i1_t1,i2_t1,i3_t1], [i1_t2,i2_t2,i3_t2], ... , [i1_tM,i2_tM,i3_tM] ]
    //index array is a collection of the form [ [r_t1,g_t1,b_t1], [r_t2,g_t2,b_t2], ... , [r_tM,g_tM,b_tM] ]
    setData(dataInfo,vertexArray,indexArray,colorArray){
	//
	this.dataInfo = dataInfo;
	//
	this.indexBuffer.bind();
	this.indexBuffer.setData(indexArray, this.glContext.STATIC_DRAW);
	console.log("Pol Index Buffer", this.indexBuffer.getSize());
	//
	this.vertexBuffer.bind();
	this.vertexBuffer.setData(vertexArray,this.glContext.STATIC_DRAW);
	console.log("Pol Vertex Buffer", this.vertexBuffer.getSize());
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

	//
	this.vertexBuffer.bind();
	this.glContext.vertexAttribPointer(
	    this.shaderProgram.getAttribLocation('aVertexPosition'),
	    numComponents,
	    type,
	    normalize,
	    stride,
	    offset);
	this.glContext.enableVertexAttribArray(this.shaderProgram.getAttribLocation('aVertexPosition'));
	
	//
	this.shaderProgram.bind();
	if(this.dataInitialized){
	    this.indexBuffer.bind();
	    //this.glContext.drawElements(this.glContext.TRIANGLES, 12, this.glContext.UNSIGNED_SHORT, 0);

	    for(var key in this.dataInfo){
	     	var elt = this.dataInfo[key];
		//
		console.log(elt);
		this.shaderProgram.setUniformVec4fv("uPolColor",elt.color);
		//debugger
		//TODO: hardcoding 2* of the unsigned_short index type
		this.glContext.drawElements(
		    this.glContext.TRIANGLES,
		    elt.numVerticesToBeRendered,
		    this.glContext.UNSIGNED_SHORT,
		    2*elt.minArrayIndex);
	    }
	}
	console.log("Data Initialized",this.dataInitialized);
    }
}
