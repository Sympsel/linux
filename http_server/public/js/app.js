// public/js/app.js
document.addEventListener('DOMContentLoaded', function() {
    console.log('Blog loaded successfully!');

    // 添加卡片点击事件
    const cards = document.querySelectorAll('.blog-card');
    cards.forEach(card => {
        card.addEventListener('click', function() {
            const link = this.querySelector('.read-more');
            if (link) {
                window.location.href = link.href;
            }
        });
    });
});
