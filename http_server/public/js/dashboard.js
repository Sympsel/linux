// public/js/dashboard.js
document.addEventListener('DOMContentLoaded', function() {
    const userMenuBtn = document.getElementById('userMenuBtn');
    const dropdownMenu = document.getElementById('dropdownMenu');
    const sidebarLinks = document.querySelectorAll('.sidebar-link');
    const dashboardSections = document.querySelectorAll('.dashboard-section');

    if (userMenuBtn && dropdownMenu) {
        userMenuBtn.addEventListener('click', function(e) {
            e.stopPropagation();
            dropdownMenu.classList.toggle('show');
        });

        document.addEventListener('click', function() {
            dropdownMenu.classList.remove('show');
        });
    }

    if (sidebarLinks.length > 0) {
        sidebarLinks.forEach(link => {
            link.addEventListener('click', function(e) {
                e.preventDefault();
                
                sidebarLinks.forEach(l => l.classList.remove('active'));
                this.classList.add('active');
                
                const targetId = this.getAttribute('data-tab');
                
                dashboardSections.forEach(section => {
                    section.style.display = 'none';
                });
                
                const targetSection = document.getElementById(targetId);
                if (targetSection) {
                    targetSection.style.display = 'block';
                }
            });
        });
    }

    const settingsForm = document.querySelector('.settings-form');
    if (settingsForm) {
        settingsForm.addEventListener('submit', function(e) {
            e.preventDefault();
            alert('设置保存功能需要后端支持');
        });

        settingsForm.addEventListener('reset', function() {
            console.log('Settings reset');
        });
    }

    const btnIcons = document.querySelectorAll('.btn-icon');
    btnIcons.forEach(btn => {
        btn.addEventListener('click', function() {
            const title = this.getAttribute('title');
            if (title === '编辑') {
                alert('编辑功能需要后端支持');
            } else if (title === '删除') {
                if (confirm('确定要删除这篇文章吗？')) {
                    alert('删除功能需要后端支持');
                }
            }
        });
    });
});
