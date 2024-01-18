$(document).ready(function () {
    // Global variables
    var aos = $('.aos.animate__animated');
    // Check if element is scrolled into view
    function isScrolledIntoView(elem) {
        var docViewTop = $(window).scrollTop();
        var docViewBottom = docViewTop + $(window).height();

        var elemTop = $(elem).offset().top;
        var elemBottom = elemTop + $(elem).height();

        return ((elemBottom <= docViewBottom) && (elemTop >= docViewTop));
    }
    // If element is scrolled into view, start animation
    $(window).scroll(function () {
        $(aos).each(function () {
            if (isScrolledIntoView(this) === true) {
                var aosAnimate = $(this).attr('data-aos');
                //console.log(aosAnimate);
                $(this).addClass(aosAnimate);
            }
        });
        // If window is scrolled to top, remove animation class
        if ($(this).scrollTop() == 0) {
            //console.log('is in top');
            $(aos).each(function () {
                var aosAnimate = $(this).attr('data-aos');
                $(this).removeClass(aosAnimate);
                //console.log(aosAnimate + ' is removed');
            }
            )
        };
    });
});