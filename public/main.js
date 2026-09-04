
const year = document.getElementById("year");
if (year) {
    year.textContent = new Date().getFullYear();
}

const themeToggle = document.getElementById("theme-toggle");
if (themeToggle) {
    themeToggle.onclick = () => {
        document.body.dataset.theme =
            document.body.dataset.theme === "dark" ? "light" : "dark";
    };
}

const grid = document.querySelector(".posts-grid");
const sortSelect = document.getElementById("sort-select");
const filterSelect = document.getElementById("filter-select");

function updateProjects() {
    let cards = Array.from(document.querySelectorAll(".card"));

    // FILTER
    const filter = filterSelect.value;
    cards.forEach(card => {
        const topic = card.dataset.topic;
        card.style.display =
            (filter === "all" || topic === filter) ? "" : "none";
    });

    // SORT
    const sort = sortSelect.value;

    let visibleCards = cards.filter(c => c.style.display !== "none");

    if (sort === "alpha") {
        visibleCards.sort((a, b) => {
            return a.querySelector(".post-title").innerText
                .localeCompare(b.querySelector(".post-title").innerText);
        });
    } else if (sort === "date") {
        visibleCards.sort((a, b) => {
            return Number(b.dataset.date) - Number(a.dataset.date);
        });
    }

    // re-append in new order
    visibleCards.forEach(card => grid.appendChild(card));
}

if (grid && sortSelect && filterSelect) {
    sortSelect.addEventListener("change", updateProjects);
    filterSelect.addEventListener("change", updateProjects);
    updateProjects();
}
