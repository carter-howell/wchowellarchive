const codeBlock = document.getElementById("walle-code");

if (codeBlock) {
  fetch("./Walle_code.cpp", { cache: "no-store" })
    .then(response => response.ok ? response.text() : Promise.reject(response.status))
    .then(text => {
      codeBlock.textContent = text;
    })
    .catch(() => {
      codeBlock.textContent = "// Unable to load code file.";
    });
}

const video = document.getElementById("walle-video");
const videoWrap = document.getElementById("walle-video-wrap");

if (video && videoWrap && "IntersectionObserver" in window) {
  video.muted = true;

  const observer = new IntersectionObserver((entries) => {
    entries.forEach((entry) => {
      if (entry.isIntersecting && entry.intersectionRatio >= 0.5) {
        video.play().catch(() => {});
      } else {
        video.pause();
      }
    });
  }, { threshold: [0.5] });

  observer.observe(videoWrap);
}
