document.addEventListener("DOMContentLoaded", function () {
    var button = document.querySelector("[data-fold-all]");
    var tree = document.querySelector("[data-directory-tree]");

    document.querySelectorAll(".folder-link").forEach(function (link) {
        link.addEventListener("click", function (event) {
            event.stopPropagation();
        });
    });

    if ((button === null) || (tree === null)) {
        return;
    }

    button.addEventListener("click", function () {
        var shouldOpen = button.getAttribute("data-state") === "closed";

        tree.querySelectorAll("details").forEach(function (item) {
            item.open = shouldOpen;
        });

        button.setAttribute("data-state", shouldOpen ? "open" : "closed");
        button.textContent = shouldOpen ? "全部折叠目录" : "全部展开目录";
    });
});
