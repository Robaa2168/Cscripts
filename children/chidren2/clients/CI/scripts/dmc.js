


console.log('DMC loaded properly');
if( window.location.href.indexOf('ciwebstage.children')!==-1){
	console.log('--ZeusTagManager--');

	 (function(d,m,c,cl,env){
        var h = d.getElementsByTagName('head')[0];
        var script=d.createElement(m);script.src='https://public.charitable.one/assets/scripts/tag_manager/staging/zeus-tagman.js?tagmanid='+c+'&cl='+cl+'&env='+env;
        script.defer=true;script.async=true;script.id='zeustagman';
        h.appendChild(script);
    })(document,'script','CI2018','CI','staging');

}else{

	//PRODUCTION ZTM Version 
	(function(d,m,c,cl,env){
        var h = d.getElementsByTagName('head')[0];
        var script=d.createElement(m);script.src='https://public.charitable.one/assets/scripts/tag_manager/production/zeus-tagman.js?tagmanid='+c+'&cl='+cl+'&env='+env;
        script.defer=true;script.async=true;script.id='zeustagman';
        h.appendChild(script);
    })(document,'script','CI2018','CI','live');
    

}