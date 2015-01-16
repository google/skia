function make_results_header_sticky( ) {
  element = $(".results-header-actions");
  var pos = element.position();
  $(window).scroll( function() {
    var windowPos = $(window).scrollTop();
    if (windowPos > pos.top) {
      element.addClass("sticky");
    } else {
      element.removeClass("sticky");
    }
  });
}
