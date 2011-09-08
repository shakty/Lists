(function ($) {
	function onLoad()
	{
		var mapDiv = $('#map-div');
		mapDiv.innerHTML = "the map will be here";
		//map = new livingscience.maps.Map(mapDiv); TODO: remove the comment whenever the API is ready
		//map.setUserID(uid); TODO: remove the comment whenever the API is ready
	}

	function onNewResultsAvailable()
	{
		map.update(); // this will trigger a reload of the visible map tiles from the livingscience server.
	}
	
	Drupal.behaviors.livingscience = {
		attach: function (context, settings) {
			$("body", context).once('onload', function () {
				onLoad();
			});
		}
	};
}(jQuery));